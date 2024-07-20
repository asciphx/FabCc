#pragma once
#include <fabCc.hh>
//Secondary Routing
namespace fc {
  fc_api(web) {
    App api;
    api["/"] = [](Req& req, Res& res)_ctx{
      res.write("Hello web!"); co_return;
    };
    api["/url"] = [](Req& req, Res& res)_ctx{
      res.write(req.url); co_return;
    };
    return api;
  }

  fc_api(user) {
    App api;
    api["/"] = [](Req& req, Res& res)_ctx{
      res.write("Hello user!"); co_return;
    };
    return api;
  }
}