#include <app.hh>
//#include <hpp/body_parser.hpp>
#include <json.hh>
#include <tp/ctx.hh>
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("主页路由被std::bind复写！");
};
int main() {
  Timer t; App app;
  app.file_type().sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
	Json x = { { "h", 23 }, { "b", false }, { "s", "xx" }, { "v", {1,2,3} }, { "o", {{"xx", 0}} } };
	res.write(x.dump());//json响应
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app["/yield"] = [](Req& req, Res& res) {
	Json x = { 1,2,3 };
	co c{ [&x](co&& c) {
	  x = json::parse(R"([{"confidence":0.974220335483551,"text":"lenovo联想","region":[[191,80],[672,80],[672,148],[191,148]]},
		{"confidence":0.6968730688095093,"text":"BY：花享湖月","region":[[250,866],[332,866],[332,885],[250,885]]}])");
	  return std::move(c);
	  } };
	res.body << x;
	c = c.yield();
	res.body << x.dump();
	res.write(res.body);
  };//协程, 使用co的yield函数来保证执行顺序
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");
  };
  app["/"] = [&](Req&, Res& res) {
	res.write("主页!!");
  };
  app["/timer"] = [&](Req&, Res& res) {
    if(t.idle()) t.setTimeout([] { exit(0); }, 6000);
	res.write("关闭服务计时器倒计时启动！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  http_serve(app, 8080);
  return 0;
}