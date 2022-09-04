
#include "uv.h"
#include "internal.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
int uv__platform_loop_init(uv_loop_t* loop) { loop->poll_fds = NULL; loop->poll_fds_used = 0; loop->poll_fds_size = 0; loop->poll_fds_iterating = 0; return 0;}void uv__platform_loop_delete(uv_loop_t* loop) { uv__free(loop->poll_fds); loop->poll_fds = NULL;}int uv__io_fork(uv_loop_t* loop) { uv__platform_loop_delete(loop); return uv__platform_loop_init(loop);}static void uv__pollfds_maybe_resize(uv_loop_t* loop) { size_t i; size_t n; struct pollfd* p; if (loop->poll_fds_used < loop->poll_fds_size) return; n = loop->poll_fds_size ? loop->poll_fds_size * 2 : 64; p = uv__reallocf(loop->poll_fds, n * sizeof(*loop->poll_fds)); if (p == NULL) abort(); loop->poll_fds = p; for (i = loop->poll_fds_size; i < n; ++i) { loop->poll_fds[i].fd = -1; loop->poll_fds[i].events = 0; loop->poll_fds[i].revents = 0; } loop->poll_fds_size = n;}static void uv__pollfds_swap(uv_loop_t* loop, size_t l, size_t r) { struct pollfd pfd; pfd = loop->poll_fds[l]; loop->poll_fds[l] = loop->poll_fds[r]; loop->poll_fds[r] = pfd;}static void uv__pollfds_add(uv_loop_t* loop, uv__io_t* w) { size_t i; struct pollfd* pe;  assert(!loop->poll_fds_iterating); for (i = 0; i < loop->poll_fds_used; ++i) { if (loop->poll_fds[i].fd == w->fd) { loop->poll_fds[i].events = w->pevents; return; } }  uv__pollfds_maybe_resize(loop); pe = &loop->poll_fds[loop->poll_fds_used++]; pe->fd = w->fd; pe->events = w->pevents;}static void uv__pollfds_del(uv_loop_t* loop, int fd) { size_t i; assert(!loop->poll_fds_iterating); for (i = 0; i < loop->poll_fds_used;) { if (loop->poll_fds[i].fd == fd) {  --loop->poll_fds_used; uv__pollfds_swap(loop, i, loop->poll_fds_used); loop->poll_fds[loop->poll_fds_used].fd = -1; loop->poll_fds[loop->poll_fds_used].events = 0; loop->poll_fds[loop->poll_fds_used].revents = 0;  if (-1 != fd) return; } else {   ++i; } }}void uv__io_poll(uv_loop_t* loop, int timeout) { sigset_t* pset; sigset_t set; uint64_t time_base; uint64_t time_diff; QUEUE* q; uv__io_t* w; size_t i; unsigned int nevents; int nfds; int have_signals; struct pollfd* pe; int fd; int user_timeout; int reset_timeout; if (loop->nfds == 0) { assert(QUEUE_EMPTY(&loop->watcher_queue)); return; }  while (!QUEUE_EMPTY(&loop->watcher_queue)) { q = QUEUE_HEAD(&loop->watcher_queue); QUEUE_REMOVE(q); QUEUE_INIT(q); w = QUEUE_DATA(q, uv__io_t, watcher_queue); assert(w->pevents != 0); assert(w->fd >= 0); assert(w->fd < (int) loop->nwatchers); uv__pollfds_add(loop, w); w->events = w->pevents; }  pset = NULL; if (loop->flags & UV_LOOP_BLOCK_SIGPROF) { pset = &set; sigemptyset(pset); sigaddset(pset, SIGPROF); } assert(timeout >= -1); time_base = loop->time; if (uv__get_internal_fields(loop)->flags & UV_METRICS_IDLE_TIME) { reset_timeout = 1; user_timeout = timeout; timeout = 0; } else { reset_timeout = 0; }  for (;;) {  if (timeout != 0) uv__metrics_set_provider_entry_time(loop); if (pset != NULL) if (pthread_sigmask(SIG_BLOCK, pset, NULL)) abort(); nfds = poll(loop->poll_fds, (nfds_t)loop->poll_fds_used, timeout); if (pset != NULL) if (pthread_sigmask(SIG_UNBLOCK, pset, NULL)) abort();  SAVE_ERRNO(uv__update_time(loop)); if (nfds == 0) { if (reset_timeout != 0) { timeout = user_timeout; reset_timeout = 0; if (timeout == -1) continue; if (timeout > 0) goto update_timeout; } assert(timeout != -1); return; } if (nfds == -1) { if (errno != EINTR) abort(); if (reset_timeout != 0) { timeout = user_timeout; reset_timeout = 0; } if (timeout == -1) continue; if (timeout == 0) return;  goto update_timeout; }  loop->poll_fds_iterating = 1;  nevents = 0; have_signals = 0;  for (i = 0; i < loop->poll_fds_used; ++i) { pe = loop->poll_fds + i; fd = pe->fd;  if (fd == -1) continue; assert(fd >= 0); assert((unsigned) fd < loop->nwatchers); w = loop->watchers[fd]; if (w == NULL) {  uv__platform_invalidate_fd(loop, fd); continue; }  pe->revents &= w->pevents | POLLERR | POLLHUP; if (pe->revents != 0) {  if (w == &loop->signal_io_watcher) { have_signals = 1; } else { uv__metrics_update_idle_time(loop); w->cb(loop, w, pe->revents); } ++nevents; } } if (reset_timeout != 0) { timeout = user_timeout; reset_timeout = 0; } if (have_signals != 0) { uv__metrics_update_idle_time(loop); loop->signal_io_watcher.cb(loop, &loop->signal_io_watcher, POLLIN); } loop->poll_fds_iterating = 0;  uv__pollfds_del(loop, -1); if (have_signals != 0) return;  if (nevents != 0) return; if (timeout == 0) return; if (timeout == -1) continue;update_timeout: assert(timeout > 0); time_diff = loop->time - time_base; if (time_diff >= (uint64_t) timeout) return; timeout -= time_diff; }}void uv__platform_invalidate_fd(uv_loop_t* loop, int fd) { size_t i; assert(fd >= 0); if (loop->poll_fds_iterating) {  for (i = 0; i < loop->poll_fds_used; ++i) if (loop->poll_fds[i].fd == fd) { loop->poll_fds[i].fd = -1; loop->poll_fds[i].events = 0; loop->poll_fds[i].revents = 0; } } else {  uv__pollfds_del(loop, fd); }}int uv__io_check_fd(uv_loop_t* loop, int fd) { struct pollfd p[1]; int rv; p[0].fd = fd; p[0].events = POLLIN; do rv = poll(p, 1, 0); while (rv == -1 && (errno == EINTR || errno == EAGAIN)); if (rv == -1) return UV__ERR(errno); if (p[0].revents & POLLNVAL) return UV_EINVAL; return 0;}