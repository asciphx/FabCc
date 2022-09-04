
#ifndef UV_LINUX_H
#define UV_LINUX_H

#define UV_PLATFORM_LOOP_FIELDS  uv__io_t inotify_read_watcher; void* inotify_watchers;  int inotify_fd; 

#define UV_PLATFORM_FS_EVENT_FIELDS  void* watchers[2]; int wd; 

#endif 
