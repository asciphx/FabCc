
#include "uv.h"
#include "internal.h"

#include <stddef.h>
#include <unistd.h>
int uv_exepath(char* buffer, size_t* size) { ssize_t n; if (buffer == NULL || size == NULL || *size == 0) return UV_EINVAL; n = *size - 1; if (n > 0) n = readlink("/proc/self/exe", buffer, n); if (n == -1) return UV__ERR(errno); buffer[n] = '\0'; *size = n; return 0;}