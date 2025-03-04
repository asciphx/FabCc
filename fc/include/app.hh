#ifndef APP_HH
#define APP_HH
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include "hh/router.hh"
//For Secondary Routing
#define fc_api(_) App _$##_()
#define fc_app(_) .sub_api(#_, fc::_$##_())
namespace fc {
  HTTP c2m(const char* m, size_t l);
  const std::string_view m2c(HTTP m);
  static fc::HashMap<std::string, std::string> RES_CACHE_MENU = {};
  static fc::HashMap<std::string, int64_t> RES_CACHE_TIME = {};
  struct App {
    App();
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
    _CTX_TASK(void) _call(char m, std::string& r, Req& request, Res& response) const;
    std::string get_cache(std::string& u);
    void set_cache(std::string& u, std::string& v, short i = CACHE_HTML_TIME_SECOND);
    //Whether to open file download, set to true will allow
    App& set_file_download(bool&& b);
    //Set not_found route;
    _FORCE_INLINE VH& default_route() { return _; }
    App& sub_api(const char* prefix, const App& subapi);
    //Serve static directory
    App serve_file(const char* r);
    //set use max mem size(MB), The remaining memory size available to the system minus the maximum size of the uploaded files
    //A quarter of this value is used as a limit, less than that will use memory, and then gradually decreases.
    //Due to certain cache settings, the minimum value can only be 512MB. But it's best not to exceed 30% of the maximum memory.
    App& set_use_max_mem(const float& f);
    //In case there are not enough file descriptors, set the maximum number of connections to prevent reaching the maximum file descriptor
    //Maybe the input bandwidth of the server is much greater than the output bandwidth
    App& file_type(const std::vector<std::string_view>& line = { "html","htm","ico","css","js","srt",
      "json","svg","png","jpg","jpeg","gif","txt","wasm","mp4","webm","mp3","wav","aac","mkv","vtt" });
    // keep-alive time seconds = idle + intvl * probes, RESk_A is the total time written. time seconds default = 6s
    App& set_keep_alive(unsigned char idle = 1, unsigned char intvl = 1, unsigned char probes = 5);
    //std::string ssl_ciphers, std::string ssl_key = "./server.key", std::string ssl_cert = "./server.crt"
    App& set_ssl(std::string ssl_ciphers, std::string ssl_key = "./server.key", std::string ssl_cert = "./server.crt");
    //Set the size of buf. 0x800 is recommended for rbNum and 0x1000 is recommended for wbNum.
    App& set_buf_size(int rbNum = 2048, int wbNum = 4096);
    void http_serve(int port = 8080, std::string ip = "", int nthreads = std::thread::hardware_concurrency());
  private:
    DRT map_;
    std::unordered_map<std::string_view, std::string_view, sv_hash, sv_key_eq> content_types;
  public:
    std::unordered_map<std::string, std::shared_ptr<file_sptr>, str_hash, str_key_eq> file_cache_;
    double USE_MAX_MEM_SIZE_MB = 400.0;
    //The default buf size is very small and is only used for running benchmarks.
    int rbNum{ 0x400 }, wbNum{ 0x800 };
  private:
    VH _ = [](Req&, Res&)_ctx{ throw err::not_found(); };
    std::string ssl_key = "", ssl_cert = "", ssl_ciphers = "";
    int k_A[3]{ 4,3,2 }; bool file_download = true;
  };
} // namespace fc
#endif