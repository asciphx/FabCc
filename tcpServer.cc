#include <tcp.hh>
#include <hpp/body_parser.hpp>
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("主页路由被std::bind复写！");
};
int main() {
  Timer t; App app; Tcp srv;
  app.sub_api("/", app.serve_file("static"));//服务文件接口
  app["/u/:id(\\d+)/:name(\\w+)"] = [](Req& req, Res& res) {//路由regex键
	res.write(req.key.get("id").str(Buffer(16) << "{\"id\": ") << ", "
	<< req.key["name"].str(Buffer(32) << "\"name\": ") << '}');
  };
  app["/json"] = [&app](Req& req, Res& res) {
	Json x = { { "h", 23 }, { "b", false }, { "s", "xx" }, { "v", {1,2,3} }, { "o", {{"xx", 0}} } };
	res.add_header(fc::RES_CT, fc::RES_AJ);
	res.write(x.dump());//json响应
  };
  app["/api/\\d/\\w+"] = [](Req& req, Res& res) {
	res.write(req.url);//regex表达式访问
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
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");//或者像下面std::bind的方式把void方法绑定都行
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("该路由已闲置1分钟，服务器即将自动关闭！！");
	  srv.exit();
	}, 60000);
	res.write("计时器倒计时启动！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  srv.router(app).timeout(9000).setTcpNoDelay(true).Start("0.0.0.0", 8080);
  return 0;
}