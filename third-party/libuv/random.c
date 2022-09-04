  
#include "uv.h"
#include "uv-common.h"

#ifdef _WIN32
# include "win/internal.h"
#else
# include "unix/internal.h"
#endif
 static int uv__random(void* buf, size_t buflen) { int rc; 
#if defined(__PASE__)
 rc = uv__random_readpath("/dev/urandom", buf, buflen);
#elif defined(_AIX) || defined(__QNX__)
 rc = uv__random_readpath("/dev/random", buf, buflen);
#elif defined(__APPLE__) || defined(__OpenBSD__) ||  (defined(__ANDROID_API__) && __ANDROID_API__ >= 28)
  rc = uv__random_getentropy(buf, buflen); if (rc == UV_ENOSYS) rc = uv__random_devurandom(buf, buflen);
#elif defined(__NetBSD__)
 rc = uv__random_sysctl(buf, buflen);
#elif defined(__FreeBSD__) || defined(__linux__)
 rc = uv__random_getrandom(buf, buflen); if (rc == UV_ENOSYS) rc = uv__random_devurandom(buf, buflen);
# if defined(__linux__)
 switch (rc) { case UV_EACCES: case UV_EIO: case UV_ELOOP: case UV_EMFILE: case UV_ENFILE: case UV_ENOENT: case UV_EPERM: rc = uv__random_sysctl(buf, buflen); break; }
# endif
#elif defined(_WIN32)
 uv__once_init(); rc = uv__random_rtlgenrandom(buf, buflen);
#else
 rc = uv__random_devurandom(buf, buflen);
#endif
 return rc; }  static void uv__random_work(struct uv__work* w) { uv_random_t* req;  req = container_of(w, uv_random_t, work_req); req->status = uv__random(req->buf, req->buflen); }  static void uv__random_done(struct uv__work* w, int status) { uv_random_t* req;  req = container_of(w, uv_random_t, work_req); uv__req_unregister(req->loop, req);  if (status == 0) status = req->status;  req->cb(req, status, req->buf, req->buflen); }  int uv_random(uv_loop_t* loop, uv_random_t* req, void *buf, size_t buflen, unsigned flags, uv_random_cb cb) { if (buflen > 0x7FFFFFFFu) return UV_E2BIG;  if (flags != 0) return UV_EINVAL;  if (cb == NULL) return uv__random(buf, buflen);  uv__req_init(loop, req, UV_RANDOM); req->loop = loop; req->status = 0; req->cb = cb; req->buf = buf; req->buflen = buflen;  uv__work_submit(loop, &req->work_req, UV__WORK_CPU, uv__random_work, uv__random_done);  return 0; } 