  
#include "uv.h"
#include "internal.h"

#include <errno.h>
#include <string.h>

#include <syscall.h>
#include <unistd.h>
 struct uv__sysctl_args { int* name; int nlen; void* oldval; size_t* oldlenp; void* newval; size_t newlen; unsigned long unused[4]; };  int uv__random_sysctl(void* buf, size_t buflen) { static int name[] = {1 , 40 , 6 }; struct uv__sysctl_args args; char uuid[16]; char* p; char* pe; size_t n;  p = buf; pe = p + buflen;  while (p < pe) { memset(&args, 0, sizeof(args));  args.name = name; args.nlen = ARRAY_SIZE(name); args.oldval = uuid; args.oldlenp = &n; n = sizeof(uuid);  
#ifdef SYS__sysctl
 if (syscall(SYS__sysctl, &args) == -1) return UV__ERR(errno);
#else
 { (void) &args; return UV_ENOSYS; }
#endif
 if (n != sizeof(uuid)) return UV_EIO; uuid[6] = uuid[14]; uuid[8] = uuid[15];  n = pe - p; if (n > 14) n = 14;  memcpy(p, uuid, n); p += n; }  return 0; } 