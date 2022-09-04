
#include "uv.h"
#include "internal.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdio.h> 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h> 

#ifdef __linux__
#include <sys/eventfd.h>
#endif
static void uv__async_send(uv_loop_t* loop);static int uv__async_start(uv_loop_t* loop);static void uv__cpu_relax(void);int uv_async_init(uv_loop_t* loop, uv_async_t* handle, uv_async_cb async_cb) { int err; err = uv__async_start(loop); if (err) return err; uv__handle_init(loop, (uv_handle_t*)handle, UV_ASYNC); handle->async_cb = async_cb; handle->pending = 0; QUEUE_INSERT_TAIL(&loop->async_handles, &handle->queue); uv__handle_start(handle); return 0;}int uv_async_send(uv_async_t* handle) { _Atomic int* pending; int expected; pending = (_Atomic int*) &handle->pending; if (atomic_load_explicit(pending, memory_order_relaxed) != 0) return 0; expected = 0; if (!atomic_compare_exchange_strong(pending, &expected, 1)) return 0; uv__async_send(handle->loop); expected = 1; if (!atomic_compare_exchange_strong(pending, &expected, 2)) abort(); return 0;}static int uv__async_spin(uv_async_t* handle) { _Atomic int* pending; int expected; int i; pending = (_Atomic int*) &handle->pending; for (;;) { for (i = 0; i < 997; ++i) { expected = 2; atomic_compare_exchange_strong(pending, &expected, 0); if (expected != 1) return expected; uv__cpu_relax(); } sched_yield(); }}void uv__async_close(uv_async_t* handle) { uv__async_spin(handle); QUEUE_REMOVE(&handle->queue); uv__handle_stop(handle);}static void uv__async_io(uv_loop_t* loop, uv__io_t* w, unsigned int events) { char buf[1024]; ssize_t r; QUEUE queue; QUEUE* q; uv_async_t* h; assert(w == &loop->async_io_watcher); for (;;) { r = read(w->fd, buf, sizeof(buf)); if (r == sizeof(buf)) continue; if (r != -1) break; if (errno == EAGAIN || errno == EWOULDBLOCK) break; if (errno == EINTR) continue; abort(); } QUEUE_MOVE(&loop->async_handles, &queue); while (!QUEUE_EMPTY(&queue)) { q = QUEUE_HEAD(&queue); h = QUEUE_DATA(q, uv_async_t, queue); QUEUE_REMOVE(q); QUEUE_INSERT_TAIL(&loop->async_handles, q); if (0 == uv__async_spin(h)) continue; if (h->async_cb == NULL) continue; h->async_cb(h); }}static void uv__async_send(uv_loop_t* loop) { const void* buf; ssize_t len; int fd; int r; buf = ""; len = 1; fd = loop->async_wfd;
#if defined(__linux__)
 if (fd == -1) { static const uint64_t val = 1; buf = &val; len = sizeof(val); fd = loop->async_io_watcher.fd; }
#endif
 do r = write(fd, buf, len); while (r == -1 && errno == EINTR); if (r == len) return; if (r == -1) if (errno == EAGAIN || errno == EWOULDBLOCK) return; abort();}static int uv__async_start(uv_loop_t* loop) { int pipefd[2]; int err; if (loop->async_io_watcher.fd != -1) return 0;
#ifdef __linux__
 err = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK); if (err < 0) return UV__ERR(errno); pipefd[0] = err; pipefd[1] = -1;
#else
 err = uv__make_pipe(pipefd, UV_NONBLOCK_PIPE); if (err < 0) return err;
#endif
 uv__io_init(&loop->async_io_watcher, uv__async_io, pipefd[0]); uv__io_start(loop, &loop->async_io_watcher, POLLIN); loop->async_wfd = pipefd[1]; return 0;}int uv__async_fork(uv_loop_t* loop) { if (loop->async_io_watcher.fd == -1) return 0; uv__async_stop(loop); return uv__async_start(loop);}void uv__async_stop(uv_loop_t* loop) { if (loop->async_io_watcher.fd == -1) return; if (loop->async_wfd != -1) { if (loop->async_wfd != loop->async_io_watcher.fd) uv__close(loop->async_wfd); loop->async_wfd = -1; } uv__io_stop(loop, &loop->async_io_watcher, POLLIN); uv__close(loop->async_io_watcher.fd); loop->async_io_watcher.fd = -1;}static void uv__cpu_relax(void) {
#if defined(__i386__) || defined(__x86_64__)
 __asm__ __volatile__ ("rep; nop" ::: "memory"); 
#elif (defined(__arm__) && __ARM_ARCH >= 7) || defined(__aarch64__)
 __asm__ __volatile__ ("yield" ::: "memory");
#elif (defined(__ppc__) || defined(__ppc64__)) && defined(__APPLE__)
 __asm volatile ("" : : : "memory");
#elif !defined(__APPLE__) && (defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__))
 __asm__ __volatile__ ("or 1,1,1; or 2,2,2" ::: "memory");
#endif
}