#include <app.hh>
#include <json.hh>
using namespace fc;
int main() {
  App app;
  app["/"] = [](Req& req, Res& res) {
    res.write("<p>Hello, world!</p>");
  };
  app["/json"] = [](Req& req, Res& res) {
    res.write(Json({"message", "Hello, World!"}).str());
  };
  app["/benchmark"] = [](Req& req, Res& res) {
    res.write("<p>Hello, world!</p>");
  };
  http_serve(app, 8080);
  return 0;
}