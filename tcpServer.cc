#include <tcp.hh>
#include <hpp/body_parser.hpp>
#include <json.hh>
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("主页路由被std::bind复写！");
};
int main() {
  Timer t; App app; Tcp srv;
  app.sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
	Json x = { { "h", 23 }, { "b", false }, { "s", "xx" }, { "v", {1,2,3} }, { "o", {{"xx", 0}} } };
	res.write(x.dump());//json响应
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 4096);
	for (auto p : bp.params) {
	  res.write(p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
  };
  app["/jdk"] = [&app](Req& req, Res& res) {
	res.write(json::parse(R"([{"confidence":0.974220335483551,"text":"lenovo联想","text_region":[[191,80],[672,80],[672,148],[191,148]]},{"confidence":0.6968730688095093,"text":"BY：花享湖月","text_region":[[250,866],[332,866],[332,885],[250,885]]}])").dump());
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("该路由已闲置1分钟，服务器即将自动关闭！！");
	  srv.exit();
	}, 60000);
	res.write("关闭服务计时器倒计时启动！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  srv.router(app).timeout(9000).setTcpNoDelay(true).Start("0.0.0.0", 8080);
  return 0;
}