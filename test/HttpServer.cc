#include"uv.h"
uv_loop_t* loop;
#define TYPE_GET(t, ptr, member) (t*)(ptr)-((size_t)&reinterpret_cast<char const volatile&>(((t*)0)->member))
typedef struct {
  uv_write_t req;
  uv_tcp_t handle;
  uv_buf_t buf;
}write_req_t;
uv_buf_t resbuf;
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}
#define RESPONSE_STR "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nhello world!你好！世界！\n"
void on_close(uv_handle_t* handle) {
  write_req_t* wr = TYPE_GET(write_req_t, handle, handle);
  //write_req_t* wr = (write_req_t*)handle->data;
  free(wr->buf.base); free(wr);
  printf("x");
}
void echo_write(uv_write_t* req, int status) {
  if (status) fprintf(stderr, "Write error %s\n", uv_strerror(status));
  write_req_t* wr = TYPE_GET(write_req_t, req, req);
  free(wr->buf.base); free(wr);
}
void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  write_req_t* wr = TYPE_GET(write_req_t, stream, req);
  //write_req_t* wr = (write_req_t*)stream->data;
  if (nread > 0) {
	uv_write((uv_write_t*)wr, stream, &resbuf, 1, echo_write);
  } else if (nread < 0) {
	printf("c");
	if (nread != UV_EOF)
	  fprintf(stderr, "Read error %s\n", uv_err_name(nread));
	uv_close((uv_handle_t*)stream, NULL);
  }
  free(buf->base);
}
void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) {
	fprintf(stderr, "New connection error %s\n", uv_strerror(status));
	return;
  }
  write_req_t* co = new write_req_t();
  int $ = uv_tcp_init(loop, &co->handle); if ($) { delete co; return; }
  if (uv_accept(server, (uv_stream_t*)&co->handle) == 0) {
	//client->handle.data = client;
	uv_tcp_keepalive(&co->handle, 1, 6);
	printf("-");
	uv_read_start((uv_stream_t*)&co->handle, alloc_buffer, read_cb);
  } else {
	uv_close((uv_handle_t*)&co->handle, on_close);
  }
}
int main(int argc, char* argv[]) {
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  resbuf.base = RESPONSE_STR;
  resbuf.len = sizeof(RESPONSE_STR);
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 8080, &addr);
  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  int r = uv_listen((uv_stream_t*)&server, 1, on_new_connection);
  if (r) {
	fprintf(stderr, "Listen error %s\n", uv_strerror(r));
	return 1;
  }
  return uv_run(loop, UV_RUN_DEFAULT);
}