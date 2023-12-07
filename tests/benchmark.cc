#include <app.hh>
#include <json.hh>
using namespace fc;
int main() {
  App app;
  app["/"] = [](Req& req, Res& res) {
    res.write("<p>Hello, world!</p>");
  };
  app["/json"] = [](Req& req, Res& res) {
    res.write(Json({ {"message", "Hello, World!"} }));
  };
  app["/plaintext"] = [](Req& req, Res& res) {
    res.set_content_type("text/plain;charset=UTF-8");
    res.write("Hello, World!");
  };
  //app["/db"] = [](Req& req, Res& res) {
  //};
  //app["/queries"] = [](Req& req, Res& res) {
  //};
  //app["/fortunes"] = [](Req& req, Res& res) {
  //};
  //app["/updates"] = [](Req& req, Res& res) {
  //};
  app.set_keep_alive(7).http_serve(8080, "0.0.0.0");
}