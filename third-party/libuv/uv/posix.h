  
#ifndef UV_POSIX_H
#define UV_POSIX_H

#define UV_PLATFORM_LOOP_FIELDS  struct pollfd* poll_fds; size_t poll_fds_used;  size_t poll_fds_size;  unsigned char poll_fds_iterating; 

#endif 
