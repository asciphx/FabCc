  
#include "uv.h"
#include "internal.h"

#ifdef __linux__

#include "linux-syscalls.h"

#define uv__random_getrandom_init() 0

#else 

#include <stddef.h>
#include <dlfcn.h>
 typedef ssize_t (*uv__getrandom_cb)(void *, size_t, unsigned); static uv__getrandom_cb uv__getrandom; static uv_once_t once = UV_ONCE_INIT; static void uv__random_getrandom_init_once(void) { uv__getrandom = (uv__getrandom_cb) dlsym(RTLD_DEFAULT, "getrandom"); } static int uv__random_getrandom_init(void) { uv_once(&once, uv__random_getrandom_init_once);  if (uv__getrandom == NULL) return UV_ENOSYS;  return 0; } 
#endif 
 int uv__random_getrandom(void* buf, size_t buflen) { ssize_t n; size_t pos; int rc;  rc = uv__random_getrandom_init(); if (rc != 0) return rc;  for (pos = 0; pos != buflen; pos += n) { do { n = buflen - pos;   if (n > 256) n = 256;  n = uv__getrandom((char *) buf + pos, n, 0); } while (n == -1 && errno == EINTR);  if (n == -1) return UV__ERR(errno);  if (n == 0) return UV_EIO; }  return 0; } 