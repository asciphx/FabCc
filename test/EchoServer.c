#include <stdio.h>
#include <uv.h>
#define DEFAULT_PORT 8080
struct sockaddr_in toaddr, addr; uv_loop_t* loop;
#define TYPE_GET(t, ptr, member) (ptr*)((char*)(t)-(size_t)&((ptr*)0)->member)
struct socket { uv_tcp_t sender; uv_timer_t timer; uv_tcp_t receiver; };
typedef struct { uv_write_t req; uv_buf_t buf; } write_req_t;
void free_write_req(uv_write_t* req) {
  write_req_t* wr = (write_req_t*)req; free(wr->buf.base); free(wr);
}
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size); buf->len = suggested_size;
}
void on_close(uv_handle_t* handle) {
  printf("close_sender\n"); struct socket* sk; sk = TYPE_GET(handle, struct socket, sender); free(sk);
}
void on_closeto(uv_handle_t* handle) {
  printf("close_receiver\n"); struct socket* sk; sk = TYPE_GET(handle, struct socket, receiver); free(sk);
}
void echo_write(uv_write_t* req, int status) {
  if (status) fprintf(stderr, "Write error %s\n", uv_strerror(status)); printf("w"); free_write_req(req);
}
void socket_timer_expire_cb(uv_timer_t* handle) {
  struct socket* sk = TYPE_GET(handle, struct socket, timer);
  uv_timer_stop(&sk->timer); printf("socket_timer_expire_cb\n");
  uv_close((uv_handle_t*)&sk->sender, NULL);
  uv_close((uv_handle_t*)&sk->receiver, NULL);
  uv_close((uv_handle_t*)handle, NULL); free(sk);
}
void echo_read(uv_stream_t* from, ssize_t nread, const uv_buf_t* buf) {
  printf("r"); uv_tcp_t* to = from->data;
  struct socket* sk = TYPE_GET(from, struct socket, sender);
  if (nread > 0) {
	write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
	req->buf = uv_buf_init(buf->base, nread);
	uv_write((uv_write_t*)req, to, &req->buf, 1, echo_write);
	return;
  }
  if (nread < 0) uv_close((uv_handle_t*)from, on_close);
  free(buf->base);
}
void echo_readto(uv_stream_t* from, ssize_t nread, const uv_buf_t* buf) {
  printf("read_to\n"); uv_tcp_t* to = from->data;
  struct socket* sk = TYPE_GET(to, struct socket, receiver);
  uv_timer_t* timer = &sk->timer;
  if (nread > 0) {
	write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
	req->buf = uv_buf_init(buf->base, nread);
	uv_write((uv_write_t*)req, to, &req->buf, 1, echo_write);
	uv_timer_start(timer, socket_timer_expire_cb, 10000, 0);//10 s
	printf("%p\n", &sk->timer);
	return;
  }
  if (nread < 0) uv_close((uv_handle_t*)to, on_closeto);
  free(buf->base);
}
void on_connect(uv_connect_t* req, int status) {
  if (status < 0) {
	fprintf(stderr, "connect failed error %s\n", uv_err_name(status)); free(req); return;
  }
  struct socket* sk = TYPE_GET(req->handle, struct socket, receiver);
  uv_stream_t* receiver = &sk->receiver;
  uv_read_start((uv_stream_t*)receiver, alloc_buffer, echo_read);
  uv_read_start((uv_stream_t*)receiver->data, alloc_buffer, echo_readto);
  free(req);
}
void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) { fprintf(stderr, "New connection error %s\n", uv_strerror(status)); return; }
  struct socket* sk = (struct socket*)malloc(sizeof(struct socket));
  server->loop->data = sk;
  uv_tcp_t* from = &sk->sender;
  uv_tcp_t* to = &sk->receiver;
  uv_timer_t* timer = &sk->timer;
  uv_tcp_init(loop, from);
  uv_timer_init(loop, timer);
  if (uv_accept(server, (uv_stream_t*)from)) {
	uv_close((uv_handle_t*)from, on_close);
  }
  uv_ip4_addr("8.129.58.72", 8080, &toaddr);
  uv_connect_t* connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
  uv_tcp_init(loop, to);
  to->data = (uv_stream_t*)from; from->data = (uv_stream_t*)to;
  uv_tcp_connect(connect_req, to, (const struct sockaddr*)&toaddr, on_connect);
}
int main() {
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  int r = uv_listen((uv_stream_t*)&server, SOMAXCONN, on_new_connection);
  if (r) {
	fprintf(stderr, "Listen error %s\n", uv_strerror(r)); return 1;
  }
  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);
  return 0;
}

