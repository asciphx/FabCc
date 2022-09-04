
#include <assert.h>

#include "uv.h"
#include "internal.h"
#include "atomicops-inl.h"
#include "handle-inl.h"
#include "req-inl.h"
void uv__async_endgame(uv_loop_t* loop, uv_async_t* handle) { if (handle->flags & UV_HANDLE_CLOSING && !handle->async_sent) { assert(!(handle->flags & UV_HANDLE_CLOSED)); uv__handle_close(handle); }}int uv_async_init(uv_loop_t* loop, uv_async_t* handle, uv_async_cb async_cb) { uv_req_t* req; uv__handle_init(loop, (uv_handle_t*) handle, UV_ASYNC); handle->async_sent = 0; handle->async_cb = async_cb; req = &handle->async_req; UV_REQ_INIT(req, UV_WAKEUP); req->data = handle; uv__handle_start(handle); return 0;}void uv__async_close(uv_loop_t* loop, uv_async_t* handle) { if (!((uv_async_t*)handle)->async_sent) { uv__want_endgame(loop, (uv_handle_t*) handle); } uv__handle_closing(handle);}int uv_async_send(uv_async_t* handle) { uv_loop_t* loop = handle->loop; if (handle->type != UV_ASYNC) { return -1; } assert(!(handle->flags & UV_HANDLE_CLOSING)); if (!uv__atomic_exchange_set(&handle->async_sent)) { POST_COMPLETION_FOR_REQ(loop, &handle->async_req); } return 0;}void uv__process_async_wakeup_req(uv_loop_t* loop, uv_async_t* handle, uv_req_t* req) { assert(handle->type == UV_ASYNC); assert(req->type == UV_WAKEUP); handle->async_sent = 0; if (handle->flags & UV_HANDLE_CLOSING) { uv__want_endgame(loop, (uv_handle_t*)handle); } else if (handle->async_cb != NULL) { handle->async_cb(handle); }}