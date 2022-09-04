  
#include "uv.h"
#include "internal.h"

#include <errno.h>
 int uv_fs_event_init(uv_loop_t* loop, uv_fs_event_t* handle) { return UV_ENOSYS; } int uv_fs_event_start(uv_fs_event_t* handle, uv_fs_event_cb cb, const char* filename, unsigned int flags) { return UV_ENOSYS; } int uv_fs_event_stop(uv_fs_event_t* handle) { return UV_ENOSYS; } void uv__fs_event_close(uv_fs_event_t* handle) { UNREACHABLE(); } 