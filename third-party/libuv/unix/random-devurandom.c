
#include "uv.h"
#include "internal.h"

#include <sys/stat.h>
#include <unistd.h>
static uv_once_t once = UV_ONCE_INIT;static int status;int uv__random_readpath(const char* path, void* buf, size_t buflen) { struct stat s; size_t pos; ssize_t n; int fd; fd = uv__open_cloexec(path, O_RDONLY); if (fd < 0) return fd; if (fstat(fd, &s)) { uv__close(fd); return UV__ERR(errno); } if (!S_ISCHR(s.st_mode)) { uv__close(fd); return UV_EIO; } for (pos = 0; pos != buflen; pos += n) { do n = read(fd, (char*) buf + pos, buflen - pos); while (n == -1 && errno == EINTR); if (n == -1) { uv__close(fd); return UV__ERR(errno); } if (n == 0) { uv__close(fd); return UV_EIO; } } uv__close(fd); return 0;}static void uv__random_devurandom_init(void) { char c;  status = uv__random_readpath("/dev/random", &c, 1);}int uv__random_devurandom(void* buf, size_t buflen) { uv_once(&once, uv__random_devurandom_init); if (status != 0) return status; return uv__random_readpath("/dev/urandom", buf, buflen);}