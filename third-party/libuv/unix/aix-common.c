
#include "uv.h"
#include "internal.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include <sys/time.h>
#include <unistd.h>

#include <procinfo.h>

#include <ctype.h>
extern char* original_exepath;extern uv_mutex_t process_title_mutex;extern uv_once_t process_title_mutex_once;extern void init_process_title_mutex_once(void);uint64_t uv__hrtime(uv_clocktype_t type) { uint64_t G = 1000000000; timebasestruct_t t; read_wall_time(&t, TIMEBASE_SZ); time_base_to_time(&t, TIMEBASE_SZ); return (uint64_t) t.tb_high * G + t.tb_low;}int uv_exepath(char* buffer, size_t* size) { int res; char args[UV__PATH_MAX]; size_t cached_len; struct procsinfo pi; if (buffer == NULL || size == NULL || *size == 0) return UV_EINVAL; uv_once(&process_title_mutex_once, init_process_title_mutex_once); uv_mutex_lock(&process_title_mutex); if (original_exepath != NULL) { cached_len = strlen(original_exepath); *size -= 1; if (*size > cached_len) *size = cached_len; memcpy(buffer, original_exepath, *size); buffer[*size] = '\0'; uv_mutex_unlock(&process_title_mutex); return 0; } uv_mutex_unlock(&process_title_mutex); pi.pi_pid = getpid(); res = getargs(&pi, sizeof(pi), args, sizeof(args)); if (res < 0) return UV_EINVAL; return uv__search_path(args, buffer, size);}