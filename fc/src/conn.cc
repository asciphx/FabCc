#include <conn.hh>

namespace fc {
  Conn::Conn() {
	ptr_ = (uv_tcp_t*)malloc(sizeof(*ptr_)); ptr_->data = this;
	rbuf = uv_buf_init((char*)malloc(BUF_SIZE), BUF_SIZE);
  }
  Conn::~Conn() {
	free(rbuf.base); rbuf.base = nullptr;
	free(ptr_); ptr_ = nullptr;
  }

}