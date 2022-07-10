#include <conn.hh>

namespace fc {
  Conn::Conn() {
	ptr_.data = this; buf_.reserve(0x1ff);
	rbuf = uv_buf_init((char*)malloc(BUF_SIZE), BUF_SIZE);
  }
  Conn::~Conn() {
	free(rbuf.base); rbuf.base = nullptr; app_ = nullptr;
  }
}
