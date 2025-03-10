#include <fabCc.hh>
using namespace fc;
int main() {
  App app;
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4","webm","mp3","wav","mkv","srt","vtt" })
    .sub_api("/", app.serve_file("static"))
    .set_keep_alive(4, 3, 2)
    .set_use_max_mem(600.0)
    .set_file_download(false);//Disable file downloads to protect certain videos or audio
  app["/get_upload"] = [](Req& req, Res& res)_ctx {
    res.write_async([] {
      auto f = fc::directory_iterator(fc::directory_ + fc::upload_path_); Json x;
      std::set<std::string_view> extentions = { "mp4", "mp3", "webm", "wav", "mkv" };
      for (auto v : f) {
        if (std::find(extentions.begin(), extentions.end(), fc::toLowerCase(v.ext)) != extentions.end()) {
          x.push_back({ {"name",v.name.substr(fc::directory_.size())}, {"size",v.size} });
        }
      } return x;
      }); co_return;
  };
  app.post("/api") = [](Req& req, Res& res)_ctx {
    BP bp(req, 1000); co_await bp.run(); std::string s;//Support for uploading files with a total size of 1000MB
    for (auto p : bp.params) {
      s << (p.key + ": ") << p.value << ", ";
    }
    s.pop_back(); s.pop_back(); res.write(s); co_return;
  };
  app.set_buf_size(0x800, 0x2000).http_serve(8080, "0.0.0.0");
}