
#include "uv.h"
#include "internal.h"

#include <stddef.h>
#include <dlfcn.h>
typedef int (*uv__getentropy_cb)(void *, size_t);static uv__getentropy_cb uv__getentropy;static uv_once_t once = UV_ONCE_INIT;static void uv__random_getentropy_init(void) { uv__getentropy = (uv__getentropy_cb) dlsym(RTLD_DEFAULT, "getentropy");}int uv__random_getentropy(void* buf, size_t buflen) { size_t pos; size_t stride; uv_once(&once, uv__random_getentropy_init); if (uv__getentropy == NULL) return UV_ENOSYS; for (pos = 0, stride = 256; pos + stride < buflen; pos += stride) if (uv__getentropy((char *) buf + pos, stride)) return UV__ERR(errno); if (uv__getentropy((char *) buf + pos, buflen - pos)) return UV__ERR(errno); return 0;}