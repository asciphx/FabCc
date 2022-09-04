
#include "uv.h"
#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#if defined(__APPLE__)
# include <spawn.h>
# include <paths.h>
# include <sys/kauth.h>
# include <sys/types.h>
# include <sys/sysctl.h>
# include <dlfcn.h>
# include <crt_externs.h>
# include <xlocale.h>
# define environ (*_NSGetEnviron())

# ifndef POSIX_SPAWN_SETSID
# define POSIX_SPAWN_SETSID 1024
# endif

#else
extern char **environ;
#endif

#if defined(__linux__) || defined(__GLIBC__)
# include <grp.h>
#endif

#if defined(__MVS__)
# include "zos-base.h"
#endif

#if defined(__APPLE__) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)

#include <sys/event.h>
#else
#define UV_USE_SIGCHLD
#endif

#ifdef UV_USE_SIGCHLD
static void uv__chld(uv_signal_t* handle, int signum) { assert(signum == SIGCHLD); uv__wait_children(handle->loop);}
#endif
void uv__wait_children(uv_loop_t* loop) { uv_process_t* process; int exit_status; int term_signal; int status; int options; pid_t pid; QUEUE pending; QUEUE* q; QUEUE* h; QUEUE_INIT(&pending); h = &loop->process_handles; q = QUEUE_HEAD(h); while (q != h) { process = QUEUE_DATA(q, uv_process_t, queue); q = QUEUE_NEXT(q);
#ifndef UV_USE_SIGCHLD
 if ((process->flags & UV_HANDLE_REAP) == 0) continue; options = 0; process->flags &= ~UV_HANDLE_REAP;
#else
 options = WNOHANG;
#endif
 do pid = waitpid(process->pid, &status, options); while (pid == -1 && errno == EINTR);
#ifdef UV_USE_SIGCHLD
 if (pid == 0) continue;
#endif
 if (pid == -1) { if (errno != ECHILD) abort(); continue; } assert(pid == process->pid); process->status = status; QUEUE_REMOVE(&process->queue); QUEUE_INSERT_TAIL(&pending, &process->queue); } h = &pending; q = QUEUE_HEAD(h); while (q != h) { process = QUEUE_DATA(q, uv_process_t, queue); q = QUEUE_NEXT(q); QUEUE_REMOVE(&process->queue); QUEUE_INIT(&process->queue); uv__handle_stop(process); if (process->exit_cb == NULL) continue; exit_status = 0; if (WIFEXITED(process->status)) exit_status = WEXITSTATUS(process->status); term_signal = 0; if (WIFSIGNALED(process->status)) term_signal = WTERMSIG(process->status); process->exit_cb(process, exit_status, term_signal); } assert(QUEUE_EMPTY(&pending));}static int uv__process_init_stdio(uv_stdio_container_t* container, int fds[2]) { int mask; int fd; mask = UV_IGNORE | UV_CREATE_PIPE | UV_INHERIT_FD | UV_INHERIT_STREAM; switch (container->flags & mask) { case UV_IGNORE: return 0; case UV_CREATE_PIPE: assert(container->data.stream != NULL); if (container->data.stream->type != UV_NAMED_PIPE) return UV_EINVAL; else return uv_socketpair(SOCK_STREAM, 0, fds, 0, 0); case UV_INHERIT_FD: case UV_INHERIT_STREAM: if (container->flags & UV_INHERIT_FD) fd = container->data.fd; else fd = uv__stream_fd(container->data.stream); if (fd == -1) return UV_EINVAL; fds[1] = fd; return 0; default: assert(0 && "Unexpected flags"); return UV_EINVAL; }}static int uv__process_open_stream(uv_stdio_container_t* container, int pipefds[2]) { int flags; int err; if (!(container->flags & UV_CREATE_PIPE) || pipefds[0] < 0) return 0; err = uv__close(pipefds[1]); if (err != 0) abort(); pipefds[1] = -1; uv__nonblock(pipefds[0], 1); flags = 0; if (container->flags & UV_WRITABLE_PIPE) flags |= UV_HANDLE_READABLE; if (container->flags & UV_READABLE_PIPE) flags |= UV_HANDLE_WRITABLE; return uv__stream_open(container->data.stream, pipefds[0], flags);}static void uv__process_close_stream(uv_stdio_container_t* container) { if (!(container->flags & UV_CREATE_PIPE)) return; uv__stream_close(container->data.stream);}static void uv__write_int(int fd, int val) { ssize_t n; do n = write(fd, &val, sizeof(val)); while (n == -1 && errno == EINTR); _exit(127);}static void uv__write_errno(int error_fd) { uv__write_int(error_fd, UV__ERR(errno));}
#if !(defined(__APPLE__) && (TARGET_OS_TV || TARGET_OS_WATCH))
static void uv__process_child_init(const uv_process_options_t* options, int stdio_count, int (*pipes)[2], int error_fd) { sigset_t signewset; int close_fd; int use_fd; int fd; int n; for (n = 1; n < 32; n += 1) { if (n == SIGKILL || n == SIGSTOP) continue; 
#if defined(__HAIKU__)
 if (n == SIGKILLTHR) continue; 
#endif
 if (SIG_ERR != signal(n, SIG_DFL)) continue; uv__write_errno(error_fd); } if (options->flags & UV_PROCESS_DETACHED) setsid(); for (fd = 0; fd < stdio_count; ++fd) { use_fd = pipes[fd][1]; if (use_fd < 0 || use_fd >= fd) continue;
#ifdef F_DUPFD_CLOEXEC 
 pipes[fd][1] = fcntl(use_fd, F_DUPFD_CLOEXEC, stdio_count);
#else
 pipes[fd][1] = fcntl(use_fd, F_DUPFD, stdio_count);
#endif
 if (pipes[fd][1] == -1) uv__write_errno(error_fd);
#ifndef F_DUPFD_CLOEXEC 
 n = uv__cloexec(pipes[fd][1], 1); if (n) uv__write_int(error_fd, n);
#endif
 } for (fd = 0; fd < stdio_count; ++fd) { close_fd = -1; use_fd = pipes[fd][1]; if (use_fd < 0) { if (fd >= 3) continue; else { uv__close_nocheckstdio(fd); use_fd = open("/dev/null", fd == 0 ? O_RDONLY : O_RDWR); close_fd = use_fd; if (use_fd < 0) uv__write_errno(error_fd); } } if (fd == use_fd) { if (close_fd == -1) { n = uv__cloexec(use_fd, 0); if (n) uv__write_int(error_fd, n); } } else { fd = dup2(use_fd, fd); } if (fd == -1) uv__write_errno(error_fd); if (fd <= 2 && close_fd == -1) uv__nonblock_fcntl(fd, 0); if (close_fd >= stdio_count) uv__close(close_fd); } if (options->cwd != NULL && chdir(options->cwd)) uv__write_errno(error_fd); if (options->flags & (UV_PROCESS_SETUID | UV_PROCESS_SETGID)) { SAVE_ERRNO(setgroups(0, NULL)); } if ((options->flags & UV_PROCESS_SETGID) && setgid(options->gid)) uv__write_errno(error_fd); if ((options->flags & UV_PROCESS_SETUID) && setuid(options->uid)) uv__write_errno(error_fd); if (options->env != NULL) environ = options->env; sigemptyset(&signewset); if (sigprocmask(SIG_SETMASK, &signewset, NULL) != 0) abort();
#ifdef __MVS__
 execvpe(options->file, options->args, environ);
#else
 execvp(options->file, options->args);
#endif
 uv__write_errno(error_fd);}
#endif

#if defined(__APPLE__)
typedef struct uv__posix_spawn_fncs_tag { struct { int (*addchdir_np)(const posix_spawn_file_actions_t *, const char *); } file_actions;} uv__posix_spawn_fncs_t;static uv_once_t posix_spawn_init_once = UV_ONCE_INIT;static uv__posix_spawn_fncs_t posix_spawn_fncs;static int posix_spawn_can_use_setsid;static void uv__spawn_init_posix_spawn_fncs(void) { posix_spawn_fncs.file_actions.addchdir_np = dlsym(RTLD_DEFAULT, "posix_spawn_file_actions_addchdir_np");}static void uv__spawn_init_can_use_setsid(void) { int which[] = {CTL_KERN, KERN_OSRELEASE}; unsigned major; unsigned minor; unsigned patch; char buf[256]; size_t len; len = sizeof(buf); if (sysctl(which, ARRAY_SIZE(which), buf, &len, NULL, 0)) return; if (3 != sscanf_l(buf, NULL, "%u.%u.%u", &major, &minor, &patch)) return; posix_spawn_can_use_setsid = (major >= 19); }static void uv__spawn_init_posix_spawn(void) { uv__spawn_init_posix_spawn_fncs(); uv__spawn_init_can_use_setsid();}static int uv__spawn_set_posix_spawn_attrs( posix_spawnattr_t* attrs, const uv__posix_spawn_fncs_t* posix_spawn_fncs, const uv_process_options_t* options) { int err; unsigned int flags; sigset_t signal_set; err = posix_spawnattr_init(attrs); if (err != 0) { return err; } if (options->flags & (UV_PROCESS_SETUID | UV_PROCESS_SETGID)) { err = ENOSYS; goto error; } flags = POSIX_SPAWN_CLOEXEC_DEFAULT | POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK; if (options->flags & UV_PROCESS_DETACHED) { if (!posix_spawn_can_use_setsid) { err = ENOSYS; goto error; } flags |= POSIX_SPAWN_SETSID; } err = posix_spawnattr_setflags(attrs, flags); if (err != 0) goto error; sigfillset(&signal_set); err = posix_spawnattr_setsigdefault(attrs, &signal_set); if (err != 0) goto error; sigemptyset(&signal_set); err = posix_spawnattr_setsigmask(attrs, &signal_set); if (err != 0) goto error; return err;error: (void) posix_spawnattr_destroy(attrs); return err;}static int uv__spawn_set_posix_spawn_file_actions( posix_spawn_file_actions_t* actions, const uv__posix_spawn_fncs_t* posix_spawn_fncs, const uv_process_options_t* options, int stdio_count, int (*pipes)[2]) { int fd; int fd2; int use_fd; int err; err = posix_spawn_file_actions_init(actions); if (err != 0) { return err; } if (options->cwd != NULL) { if (posix_spawn_fncs->file_actions.addchdir_np == NULL) { err = ENOSYS; goto error; } err = posix_spawn_fncs->file_actions.addchdir_np(actions, options->cwd); if (err != 0) goto error; }  for (fd = 0; fd < stdio_count; ++fd) { use_fd = pipes[fd][1]; if (use_fd < 0 || use_fd >= fd) continue; use_fd = stdio_count; for (fd2 = 0; fd2 < stdio_count; ++fd2) {  if (pipes[fd2][1] == use_fd) { ++use_fd; fd2 = 0; } } err = posix_spawn_file_actions_adddup2( actions, pipes[fd][1], use_fd); assert(err != ENOSYS); if (err != 0) goto error; pipes[fd][1] = use_fd; } for (fd = 0; fd < stdio_count; ++fd) { use_fd = pipes[fd][1]; if (use_fd < 0) { if (fd >= 3) continue; else { err = posix_spawn_file_actions_addopen( actions, fd, "/dev/null", fd == 0 ? O_RDONLY : O_RDWR, 0); assert(err != ENOSYS); if (err != 0) goto error; continue; } } if (fd == use_fd) err = posix_spawn_file_actions_addinherit_np(actions, fd); else err = posix_spawn_file_actions_adddup2(actions, use_fd, fd); assert(err != ENOSYS); if (err != 0) goto error; uv__nonblock_fcntl(use_fd, 0); } for (fd = 0; fd < stdio_count; ++fd) { use_fd = pipes[fd][1]; if (use_fd < stdio_count) continue; for (fd2 = 0; fd2 < fd; ++fd2) { if (pipes[fd2][1] == use_fd) break; } if (fd2 < fd) continue; err = posix_spawn_file_actions_addclose(actions, use_fd); assert(err != ENOSYS); if (err != 0) goto error; } return 0;error: (void) posix_spawn_file_actions_destroy(actions); return err;}char* uv__spawn_find_path_in_env(char** env) { char** env_iterator; const char path_var[] = "PATH="; for (env_iterator = env; *env_iterator != NULL; ++env_iterator) { if (strncmp(*env_iterator, path_var, sizeof(path_var) - 1) == 0) { return *env_iterator + sizeof(path_var) - 1; } } return NULL;}static int uv__spawn_resolve_and_spawn(const uv_process_options_t* options, posix_spawnattr_t* attrs, posix_spawn_file_actions_t* actions, pid_t* pid) { const char *p; const char *z; const char *path; size_t l; size_t k; int err; int seen_eacces; path = NULL; err = -1; seen_eacces = 0; if (options->file == NULL) return ENOENT; char** env = environ; if (options->env != NULL) env = options->env; if (strchr(options->file, '/') != NULL) { do err = posix_spawn(pid, options->file, actions, attrs, options->args, env); while (err == EINTR); return err; } path = uv__spawn_find_path_in_env(env);  if (path == NULL) path = _PATH_DEFPATH; k = strnlen(options->file, NAME_MAX + 1); if (k > NAME_MAX) return ENAMETOOLONG; l = strnlen(path, PATH_MAX - 1) + 1; for (p = path;; p = z) { char b[PATH_MAX + NAME_MAX]; z = strchr(p, ':'); if (!z) z = p + strlen(p); if ((size_t)(z - p) >= l) { if (!*z++) break; continue; } memcpy(b, p, z - p); b[z - p] = '/'; memcpy(b + (z - p) + (z > p), options->file, k + 1); do err = posix_spawn(pid, b, actions, attrs, options->args, env); while (err == EINTR); switch (err) { case EACCES: seen_eacces = 1; break; case ENOENT: case ENOTDIR: break; default: return err; } if (!*z++) break; } if (seen_eacces) return EACCES; return err;}static int uv__spawn_and_init_child_posix_spawn( const uv_process_options_t* options, int stdio_count, int (*pipes)[2], pid_t* pid, const uv__posix_spawn_fncs_t* posix_spawn_fncs) { int err; posix_spawnattr_t attrs; posix_spawn_file_actions_t actions; err = uv__spawn_set_posix_spawn_attrs(&attrs, posix_spawn_fncs, options); if (err != 0) goto error; err = uv__spawn_set_posix_spawn_file_actions(&actions, posix_spawn_fncs, options, stdio_count, pipes); if (err != 0) { (void) posix_spawnattr_destroy(&attrs); goto error; } err = uv__spawn_resolve_and_spawn(options, &attrs, &actions, pid); assert(err != ENOSYS); (void) posix_spawn_file_actions_destroy(&actions); (void) posix_spawnattr_destroy(&attrs);error: return UV__ERR(err);}
#endif
static int uv__spawn_and_init_child_fork(const uv_process_options_t* options, int stdio_count, int (*pipes)[2], int error_fd, pid_t* pid) { sigset_t signewset; sigset_t sigoldset; sigfillset(&signewset); sigdelset(&signewset, SIGKILL); sigdelset(&signewset, SIGSTOP); sigdelset(&signewset, SIGTRAP); sigdelset(&signewset, SIGSEGV); sigdelset(&signewset, SIGBUS); sigdelset(&signewset, SIGILL); sigdelset(&signewset, SIGSYS); sigdelset(&signewset, SIGABRT); if (pthread_sigmask(SIG_BLOCK, &signewset, &sigoldset) != 0) abort(); *pid = fork(); if (*pid == 0) { uv__process_child_init(options, stdio_count, pipes, error_fd); abort(); } if (pthread_sigmask(SIG_SETMASK, &sigoldset, NULL) != 0) abort(); if (*pid == -1) return UV__ERR(errno); return 0;}static int uv__spawn_and_init_child( uv_loop_t* loop, const uv_process_options_t* options, int stdio_count, int (*pipes)[2], pid_t* pid) { int signal_pipe[2] = { -1, -1 }; int status; int err; int exec_errorno; ssize_t r;
#if defined(__APPLE__)
 uv_once(&posix_spawn_init_once, uv__spawn_init_posix_spawn);  err = uv__spawn_and_init_child_posix_spawn(options, stdio_count, pipes, pid, &posix_spawn_fncs); if (err != UV_ENOSYS) return err;
#endif
  err = uv__make_pipe(signal_pipe, 0); if (err) return err; uv_rwlock_wrlock(&loop->cloexec_lock); err = uv__spawn_and_init_child_fork(options, stdio_count, pipes, signal_pipe[1], pid); uv_rwlock_wrunlock(&loop->cloexec_lock); uv__close(signal_pipe[1]); if (err == 0) { do r = read(signal_pipe[0], &exec_errorno, sizeof(exec_errorno)); while (r == -1 && errno == EINTR); if (r == 0) ; else if (r == sizeof(exec_errorno)) { do err = waitpid(*pid, &status, 0); while (err == -1 && errno == EINTR); assert(err == *pid); err = exec_errorno; } else if (r == -1 && errno == EPIPE) { do err = waitpid(*pid, &status, 0); while (err == -1 && errno == EINTR); assert(err == *pid); err = UV_EPIPE; } else abort(); } uv__close_nocheckstdio(signal_pipe[0]); return err;}int uv_spawn(uv_loop_t* loop, uv_process_t* process, const uv_process_options_t* options) {
#if defined(__APPLE__) && (TARGET_OS_TV || TARGET_OS_WATCH)
 return UV_ENOSYS;
#else
 int pipes_storage[8][2]; int (*pipes)[2]; int stdio_count; pid_t pid; int err; int exec_errorno; int i; assert(options->file != NULL); assert(!(options->flags & ~(UV_PROCESS_DETACHED | UV_PROCESS_SETGID | UV_PROCESS_SETUID | UV_PROCESS_WINDOWS_HIDE | UV_PROCESS_WINDOWS_HIDE_CONSOLE | UV_PROCESS_WINDOWS_HIDE_GUI | UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS))); uv__handle_init(loop, (uv_handle_t*)process, UV_PROCESS); QUEUE_INIT(&process->queue); process->status = 0; stdio_count = options->stdio_count; if (stdio_count < 3) stdio_count = 3; err = UV_ENOMEM; pipes = pipes_storage; if (stdio_count > (int) ARRAY_SIZE(pipes_storage)) pipes = uv__malloc(stdio_count * sizeof(*pipes)); if (pipes == NULL) goto error; for (i = 0; i < stdio_count; ++i) { pipes[i][0] = -1; pipes[i][1] = -1; } for (i = 0; i < options->stdio_count; ++i) { err = uv__process_init_stdio(options->stdio + i, pipes[i]); if (err) goto error; }
#ifdef UV_USE_SIGCHLD
 uv_signal_start(&loop->child_watcher, uv__chld, SIGCHLD);
#endif
 exec_errorno = uv__spawn_and_init_child(loop, options, stdio_count, pipes, &pid);
#if 0
 if (exec_errorno != 0) goto error;
#endif
 if (exec_errorno == 0) {
#ifndef UV_USE_SIGCHLD
 struct kevent event; EV_SET(&event, pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, 0); if (kevent(loop->backend_fd, &event, 1, NULL, 0, NULL)) { if (errno != ESRCH) abort(); process->flags |= UV_HANDLE_REAP; loop->flags |= UV_LOOP_REAP_CHILDREN; }
#endif
 process->pid = pid; process->exit_cb = options->exit_cb; QUEUE_INSERT_TAIL(&loop->process_handles, &process->queue); uv__handle_start(process); } for (i = 0; i < options->stdio_count; ++i) { err = uv__process_open_stream(options->stdio + i, pipes[i]); if (err == 0) continue; while (i--) uv__process_close_stream(options->stdio + i); goto error; } if (pipes != pipes_storage) uv__free(pipes); return exec_errorno;error: if (pipes != NULL) { for (i = 0; i < stdio_count; ++i) { if (i < options->stdio_count) if (options->stdio[i].flags & (UV_INHERIT_FD | UV_INHERIT_STREAM)) continue; if (pipes[i][0] != -1) uv__close_nocheckstdio(pipes[i][0]); if (pipes[i][1] != -1) uv__close_nocheckstdio(pipes[i][1]); } if (pipes != pipes_storage) uv__free(pipes); } return err;
#endif
}int uv_process_kill(uv_process_t* process, int signum) { return uv_kill(process->pid, signum);}int uv_kill(int pid, int signum) { if (kill(pid, signum)) {
#if defined(__MVS__)
 siginfo_t infop; if (errno == EPERM && waitid(P_PID, pid, &infop, WNOHANG | WNOWAIT | WEXITED) == 0) return 0;
#endif
 return UV__ERR(errno); } else return 0;}void uv__process_close(uv_process_t* handle) { QUEUE_REMOVE(&handle->queue); uv__handle_stop(handle); if (QUEUE_EMPTY(&handle->loop->process_handles)) uv_signal_stop(&handle->loop->child_watcher);}