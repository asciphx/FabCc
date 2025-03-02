#include <app.hh>
#include <json.hh>
using namespace fc;
int main() {
  App app;
  app["/"] = [](Req& req, Res& res)_ctx {
    res.write("<p>Hello, world!</p>"); co_return;
  };
  app["/json"] = [](Req& req, Res& res)_ctx {
    res.write(Json({ {"message", "Hello, World!"} })); co_return;
  };
  app["/plaintext"] = [](Req& req, Res& res)_ctx {
    res.set_content_type("text/plain;charset=UTF-8");
    res.write("Hello, World!"); co_return;
  };
  //app["/db"] = [](Req& req, Res& res)_ctx {
  //};
  //app["/queries"] = [](Req& req, Res& res)_ctx {
  //};
  //app["/fortunes"] = [](Req& req, Res& res)_ctx {
  //};
  //app["/updates"] = [](Req& req, Res& res)_ctx {
  //};
  app.set_buf_size(0x400, 0x400).set_keep_alive(7).http_serve(8080, "0.0.0.0");
}