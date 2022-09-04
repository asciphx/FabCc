
#ifndef UV_BSD_H
#define UV_BSD_H

#define UV_PLATFORM_FS_EVENT_FIELDS  uv__io_t event_watcher; 

#define UV_IO_PRIVATE_PLATFORM_FIELDS  int rcount;  int wcount; 

#define UV_HAVE_KQUEUE 1

#endif 
