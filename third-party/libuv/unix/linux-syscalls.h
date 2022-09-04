  
#ifndef UV_LINUX_SYSCALL_H_
#define UV_LINUX_SYSCALL_H_

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
 struct uv__statx_timestamp { int64_t tv_sec; uint32_t tv_nsec; int32_t unused0; }; struct uv__statx { uint32_t stx_mask; uint32_t stx_blksize; uint64_t stx_attributes; uint32_t stx_nlink; uint32_t stx_uid; uint32_t stx_gid; uint16_t stx_mode; uint16_t unused0; uint64_t stx_ino; uint64_t stx_size; uint64_t stx_blocks; uint64_t stx_attributes_mask; struct uv__statx_timestamp stx_atime; struct uv__statx_timestamp stx_btime; struct uv__statx_timestamp stx_ctime; struct uv__statx_timestamp stx_mtime; uint32_t stx_rdev_major; uint32_t stx_rdev_minor; uint32_t stx_dev_major; uint32_t stx_dev_minor; uint64_t unused1[14]; }; ssize_t uv__preadv(int fd, const struct iovec *iov, int iovcnt, int64_t offset); ssize_t uv__pwritev(int fd, const struct iovec *iov, int iovcnt, int64_t offset); int uv__dup3(int oldfd, int newfd, int flags); ssize_t uv__fs_copy_file_range(int fd_in,  off_t* off_in,  int fd_out,  off_t* off_out,  size_t len,  unsigned int flags); int uv__statx(int dirfd, const char* path, int flags, unsigned int mask, struct uv__statx* statxbuf); ssize_t uv__getrandom(void* buf, size_t buflen, unsigned flags); 
#endif 
