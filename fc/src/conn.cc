#include <conn.hh>
#include <tcp.hh>
namespace fc {
  void Conn::epoll_mod(socket_type fd, int flags) { reactor->epoll_mod(fd, flags); }
  int Conn::read(char* buf, int max_size) {
    int count = read_impl(buf, max_size);
    while (count <= 0) {
      if ((count < 0 && errno != EAGAIN) || count == 0) return int(0);
      sink = sink.yield();
      count = read_impl(buf, max_size);
    }
    return count;
  };
  bool Conn::write(const char* buf, int size) {
    const char* end = buf + size;
    int count = write_impl(buf, int(end - buf));
    if (count > 0) buf += count;
    while (buf != end) {
      if ((count < 0 && errno != EAGAIN) || count == 0) return false;
      sink = sink.yield();
      count = write_impl(buf, int(end - buf));
      if (count > 0) buf += count;
    }
    return true;
  };
  void Conn::async(std::function<void()>&& func){
    func();
  };
  int Conn::shut(socket_type fd, sd_type type) { return ::shutdown(fd, type); }
  int Conn::shut(sd_type type) { return ::shutdown(this->socket_fd, type); }
}
