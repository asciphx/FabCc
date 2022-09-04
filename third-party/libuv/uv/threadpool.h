/* * This file is private to libuv. It provides common functionality to both * Windows and Unix backends. */
#ifndef UV_THREADPOOL_H_
#define UV_THREADPOOL_H_
struct uv__work { void (*work)(struct uv__work *w); void (*done)(struct uv__work *w, int status); struct uv_loop_s* loop; void* wq[2];};
#endif 
