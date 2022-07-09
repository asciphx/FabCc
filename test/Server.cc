#include"uv.h"
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}
typedef struct {
  uv_write_t req;
  uv_buf_t buf;
}write_req_t;
void on_close(uv_handle_t* handle) {
  free(handle);
}
void echo_write(uv_write_t* req, int status) {
  if (status) {
	fprintf(stderr, "Write error %s\n", uv_strerror(status));
	return;
  }
  write_req_t* wr = (write_req_t*)req;
	  //printf(" nread < 0 %s", wr->buf.base);
  free(wr->buf.base);
  free(wr);
}
void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread > 0) {
	buf->base[nread] = '\0';
	//printf("%s\n", buf->base);
	write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
	req->buf.base = (char*)malloc(78);
	req->buf.len = 78;
	memcpy(req->buf.base, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nhello world\n", 78);
	uv_write((uv_write_t*)req, stream, &req->buf, 1, echo_write);
  }
  if (buf->base != NULL) {
	free(buf->base);
  }
}
void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) {
	fprintf(stderr, "New connection error %s\n", uv_strerror(status));
	return;
  }
  uv_loop_t* loop = uv_default_loop();
  uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if (uv_accept(server, (uv_stream_t*)client) == 0) {
	uv_read_start((uv_stream_t*)client, alloc_buffer, read_cb);
  } else {
	uv_close((uv_handle_t*)client, on_close);
  }
}
int main(int argc, char* argv[]) {
  uv_loop_t* loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 8080, &addr);
  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  int DEFAULT_PORT = 0;
  int r = uv_listen((uv_stream_t*)&server, 1, on_new_connection);
  if (r) {
	fprintf(stderr, "Listen error %s\n", uv_strerror(r));
	return 1;
  }
  return uv_run(loop, UV_RUN_DEFAULT);
}