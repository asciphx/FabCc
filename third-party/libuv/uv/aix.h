
#ifndef UV_AIX_H
#define UV_AIX_H

#define UV_PLATFORM_LOOP_FIELDS  int fs_fd; 

#define UV_PLATFORM_FS_EVENT_FIELDS  uv__io_t event_watcher;  char *dir_filename; 

#endif 
