#include <app.hh>
using namespace fc;
int main() {
  App api;
  api["/"] = [](Req& req, Res& res) {
    res.set_header("Content-Type", "text/plain");
    res.write("Hello, World!");
  };
  api.http_serve(80);
  App app;
  app["/"] = [](Req& req, Res& res) {
    res.write("<p>Hello, world!</p>");
  };
  app.http_serve(8080);
  App::run();
}