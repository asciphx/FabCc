#include <fabCc.hh>
using namespace fc;
int main() {
  App app;
  app["/"] = [](Req& req, Res& res) {
    res.write("<p>Hello, world!</p>");
  };
  system("openssl req -new -newkey rsa:2048 -x509 -sha256 -days 365 -nodes -out ./server.crt -keyout ./server.key -subj \"/CN=localhost\"");
  app.set_ssl("ALL:!NULL").http_serve(443, "0.0.0.0");
}