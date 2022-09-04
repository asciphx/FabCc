
#ifndef UV_SUNOS_H
#define UV_SUNOS_H

#include <sys/port.h>
#include <port.h>

#define UV_PLATFORM_LOOP_FIELDS  uv__io_t fs_event_watcher; int fs_fd; 

#if defined(PORT_SOURCE_FILE)

# define UV_PLATFORM_FS_EVENT_FIELDS file_obj_t fo; int fd; 

#endif 

#endif 
