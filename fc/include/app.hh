#ifndef APP_HH
#define APP_HH
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include "hh/router.hh"
namespace fc {
  HTTP c2m(const char* m, size_t l);
  const char* m2c(HTTP m);
  struct App {
    App(); VH _ = [](Req&, Res&) { throw err::not_found(); };
    VH& operator[](const char* r);
    VH& del(const char* r);
    VH& get(const char* r = "/");//"\0"//with skip the last / of the url.
    VH& post(const char* r);
    VH& put(const char* r);
    VH& patch(const char* r);
    //template <typename Adaptor> //websocket
    //void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
    ///Process the Req and generate a Res for it
    std::string _print_routes();
    void _call(char m, std::string& r, Req& request, Res& response) const;
    DRT map_;
    double USE_MAX_MEM_SIZE_MB = 400.0;
    int k_A[3] = { 4,3,2 };
    bool file_download = true;
    std::string get_cache(std::string& u);
    void set_cache(std::string& u, std::string& v, short i = CACHE_HTML_TIME_SECOND);
    //Whether to open file download, set to true will allow
    App& set_file_download(bool&& b);
    //Set not_found route;
    _FORCE_INLINE VH& default_route() { return _; }
    std::unordered_map<std::string, std::shared_ptr<file_sptr>> file_cache_;
    std::unordered_map<std::string_view, std::string_view> content_types;
    App& sub_api(const char* prefix, const App& subapi);
    //Serve static directory
    App serve_file(const char* r);
    //set use max mem size(MB), The remaining memory size available to the system minus the maximum size of the uploaded files
    //A quarter of this value is used as a limit, less than that will use memory, and then gradually decreases.
    App& set_use_max_mem(const float& f);
    //In case there are not enough file descriptors, set the maximum number of connections to prevent reaching the maximum file descriptor
    //Maybe the input bandwidth of the server is much greater than the output bandwidth
    App& file_type(const std::vector<std::string_view>& line = { "html","htm","ico","css","js","srt",
      "json","svg","png","jpg","jpeg","gif","txt","wasm","mp4","webm","mp3","wav","aac","mkv","vtt" });
    // keep-alive time seconds = (idle + intvl * probes) >> 1, RESk_A is the total time written. time seconds default = 6s
    App& set_keep_alive(int idle, int intvl = 1, unsigned char probes = 10);
    void http_serve(int port = 8080, std::string ip = "", int nthreads = std::thread::hardware_concurrency());
  };
} // namespace fc
#endif
