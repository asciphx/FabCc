#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <h/llhttp.h>
#define BUF_SIZE 0x10000
uv_loop_t* loop;
struct client_t {
  uv_write_t req;
  uv_buf_t rbuf;
  uv_tcp_t handle;
  llhttp__internal_t parser;
  client_t() {
	handle.data = this;
	rbuf.base = (char*)malloc(BUF_SIZE), rbuf.len = BUF_SIZE;
  };
  ~client_t() {
	free(rbuf.base), rbuf.base = nullptr;
  }
};
static llhttp_settings_t settings;
uv_buf_t resbuf;
#define RESPONSE_STR  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nhello world\n"
void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  client_t* c = (client_t*)handle->data; *buf = c->rbuf;
}
void on_close(uv_handle_t* handle) { client_t* c = (client_t*)handle->data; delete c; }
void after_write(uv_write_t* wr, int status) { uv_close((uv_handle_t*)wr->handle, on_close); }
int on_headers_complete(llhttp__internal_t* parser) {
  client_t* client = (client_t*)parser->data;
  uv_write(&client->req, (uv_stream_t*)&client->handle, &resbuf, 1, NULL);
  return 1;
};
void on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  client_t* client = (client_t*)handle->data;
  if (nread > 0) {
	int failed = llhttp__internal_execute(&client->parser, buf->base, buf->base + nread);
	if (failed) { uv_close((uv_handle_t*)&client->handle, on_close); return; }
	//write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
	//req->buf = uv_buf_init(buf->base, nread);
	return;
  }
  if (nread < 0) {
	if (nread != UV_EOF) fprintf(stderr, "Read error %s\n", uv_err_name(nread));
	uv_close((uv_handle_t*)handle, on_close);
  }
}
void on_connection(uv_stream_t* q, int status) {
  client_t* client = new client_t();
  int $ = uv_tcp_init(loop, &client->handle); if ($) { delete client; return; }
  if (uv_accept(q, (uv_stream_t*)&client->handle) == 0) {
	llhttp__internal_init(&client->parser); client->parser.type = (uint8_t)HTTP_REQUEST;
	client->parser.settings = (void*)&settings;
	client->parser.data = client; client->handle.data = client;
	uv_tcp_keepalive(&client->handle, 1, 6);
	uv_read_start((uv_stream_t*)&client->handle, on_alloc, on_read);
  } else {
	printf("uv_close\n");
	uv_close((uv_handle_t*)&client->handle, NULL);
  }
}
int main() {
  settings.on_headers_complete = on_headers_complete;
  resbuf.base = RESPONSE_STR;
  resbuf.len = sizeof(RESPONSE_STR);
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  struct sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 8080, &addr);
  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  int DEFAULT_PORT = 0;
  int r = uv_listen((uv_stream_t*)&server, 128, on_connection);
  if (r) {
	fprintf(stderr, "Listen error %s\n", uv_strerror(r));
	return 1;
  }
  return uv_run(loop, UV_RUN_DEFAULT);
}
