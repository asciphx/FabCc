
#ifndef UV_WIN_STREAM_INL_H_
#define UV_WIN_STREAM_INL_H_

#include <assert.h>

#include "uv.h"
#include "internal.h"
#include "handle-inl.h"
#include "req-inl.h"
INLINE static void uv__stream_init(uv_loop_t* loop, uv_stream_t* handle, uv_handle_type type) { uv__handle_init(loop, (uv_handle_t*) handle, type); handle->write_queue_size = 0; handle->activecnt = 0; handle->stream.conn.shutdown_req = NULL; handle->stream.conn.write_reqs_pending = 0; UV_REQ_INIT(&handle->read_req, UV_READ); handle->read_req.event_handle = NULL; handle->read_req.wait_handle = INVALID_HANDLE_VALUE; handle->read_req.data = handle;}INLINE static void uv__connection_init(uv_stream_t* handle) { handle->flags |= UV_HANDLE_CONNECTION;}
#endif 
