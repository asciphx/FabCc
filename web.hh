#pragma once
#include <fabCc.hh>
//Secondary Routing
namespace fc {
  fc_api(web) {
    App app;
    app["/"] = [](Req& req, Res& res)_ctx{
      res.write("Hello web!"); co_return;
    };
    app["/url"] = [](Req& req, Res& res)_ctx{
      res.write(req.url); co_return;
    };
    return app;
  }

  fc_api(user) {
    App app;
    app["/"] = [](Req& req, Res& res)_ctx{
      res.write("Hello user!"); co_return;
    };
    return app;
  }
}