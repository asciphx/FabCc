#include <tcp.hh>
#include <iostream>
void main() {
  fc::Tcp srv;
  //启动服务器
  srv.Start("127.0.0.1");
}