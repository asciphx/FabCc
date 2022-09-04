
#ifndef UV_DARWIN_H
#define UV_DARWIN_H

#if defined(__APPLE__) && defined(__MACH__)
# include <mach/mach.h>
# include <mach/task.h>
# include <mach/semaphore.h>
# include <TargetConditionals.h>
# define UV_PLATFORM_SEM_T semaphore_t
#endif

#define UV_IO_PRIVATE_PLATFORM_FIELDS  int rcount;  int wcount; 

#define UV_PLATFORM_LOOP_FIELDS  uv_thread_t cf_thread; void* _cf_reserved;  void* cf_state;  uv_mutex_t cf_mutex; uv_sem_t cf_sem; void* cf_signals[2]; 

#define UV_PLATFORM_FS_EVENT_FIELDS  uv__io_t event_watcher;  char* realpath;  int realpath_len;  int cf_flags;  uv_async_t* cf_cb; void* cf_events[2];  void* cf_member[2];  int cf_error;  uv_mutex_t cf_mutex; 

#define UV_STREAM_PRIVATE_PLATFORM_FIELDS  void* select; 

#define UV_HAVE_KQUEUE 1

#endif 
