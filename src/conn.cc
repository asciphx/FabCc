#include <conn.hh>

namespace fc {
  Conn::Conn(fc::llParser* p):parser_(p) {
	ptr_ = (uv_tcp_t*)malloc(sizeof(*ptr_)); ptr_->data = this;
	buf = uv_buf_init((char*)malloc(BUF_SIZE), BUF_SIZE);
  }
  Conn::~Conn() {
	free(buf.base); buf.base = nullptr; buf.len = 0; free(ptr_); ptr_ = nullptr; parser_ = nullptr;
  }

}