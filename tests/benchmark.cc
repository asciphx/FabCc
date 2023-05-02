#include <app.hh>
#include <json.hh>
using namespace fc;
int main() {
  App app;
  app["/json"] = [](Req& req, Res& res) {
    res.write(Json({ {"message", "Hello, World!"} }).str());
  };
  app["/plaintext"] = [](Req& req, Res& res) {
    res.set_header("Content-Type", "text/plain");
    res.write("Hello, World!");
  };
  app["/db"] = [](Req& req, Res& res) {
  };
  app["/queries"] = [](Req& req, Res& res) {
  };
  app["/fortunes"] = [](Req& req, Res& res) {
  };
  app["/updates"] = [](Req& req, Res& res) {
  };
  app.http_serve(8080);
  return 0;
}