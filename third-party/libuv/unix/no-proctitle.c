
#include "uv.h"
#include "internal.h"

#include <errno.h>
#include <stddef.h>
char** uv_setup_args(int argc, char** argv) { return argv;}void uv__process_title_cleanup(void) {}int uv_set_process_title(const char* title) { return 0;}int uv_get_process_title(char* buffer, size_t size) { if (buffer == NULL || size == 0) return UV_EINVAL; buffer[0] = '\0'; return 0;}