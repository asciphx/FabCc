#include <fabCc.hh>
using namespace fc;
int main() {
  App app;
  app["/"] = [](Req& req, Res& res)_ctx {
    res.write("<p>Hello, world!</p>"); co_return;;
  };
  system("openssl req -new -newkey rsa:4096 -x509 -sha256 -days 365 -nodes -out ./server.crt -keyout ./server.key -subj \"/CN=localhost\"");
  app.set_ssl("ALL:!NULL").http_serve(443, "0.0.0.0");
}