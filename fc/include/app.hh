#pragma once
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include <h/common.h>
#include <h/any_types.h>
#include "router.hh"
#include <buf.hh>
#include <directory.hh>
#include <http_error.hh>
#include <file_sptr.hh>
namespace fc {
  char c2m(const char* m);
  const char* m2c(HTTP m);
  struct App {
	App();
	VH& operator[](const char* r);
	VH& del(const char* r);
	VH& get(const char* r = "/");//"\0"//with skip the last / of the url.
	VH& post(const char* r);
	VH& put(const char* r);
	//VH& patch(const char* r);
	//template <typename Adaptor> //websocket
	//void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
	///Process the Req and generate a Res for it
	Buf _print_routes();
	void _call(char m, fc::Buf& r, Req& request, Res& response) const;
	DRT map_;
	std::unordered_map<std::string, std::shared_ptr<file_sptr>> file_cache_;
	void sub_api(const char* prefix, const App& subapi);
	//Serve static directory
	App serve_file(const char* r);
	App& file_type(const std::vector<std::string_view>& line = { "html","ico","css","js","json","svg","png","jpg","gif","txt","wasm" });
  };
  static void http_serve(App& api, int port = 8080, int nthreads = std::thread::hardware_concurrency(), std::string ip = "") {
	std::function<void(Conn&)> make_http_processor = [&api](Conn& fiber) {
	  try {
		input_buffer rb;
		bool socket_is_valid = true;
		Ctx ctx = Ctx(rb, fiber);
		ctx.socket_fd = fiber.socket_fd;
		while (true) {
		  ctx.is_body_read_ = false;
		  ctx.header_lines.clear();
		  ctx.header_lines.reserve(100);
		  // Read until there is a complete header.
		  int header_start = rb.cursor;
		  int header_end = rb.cursor;
		  assert(header_start >= 0);
		  assert(header_end >= 0);
		  assert(ctx.header_lines.size() == 0);
		  ctx.add_header_line(rb.data() + header_end);
		  assert(ctx.header_lines.size() == 1);
		  bool complete_header = false;
		  if (rb.empty())
			if (!rb.read_more(fiber))
			  return;
		  const char* cur = rb.data() + header_end;
		  const char* rbend = rb.data() + rb.end - 3;
		  while (!complete_header) {
			// Look for end of header && save header lines.
#if 0
		  // Memchr optimization. Does not seem to help but I can't find why.
			while (cur < rbend) {
			  cur = (const char*)memchr(cur, '\r', 1 + rbend - cur);
			  if (!cur) {
				cur = rbend + 1;
				break;
			  }
			  if (cur[1] == '\n') { // \n already checked by memchr.
#else
			while ((cur - rb.data()) < rb.end - 3) {
			  if (cur[0] == '\r' && cur[1] == '\n') {
#endif
				cur += 2; // skip \r\n
				ctx.add_header_line(cur);
				// If we read \r\n twice the header is complete.
				if (cur[0] == '\r' && cur[1] == '\n') {
				  complete_header = true;
				  cur += 2; // skip \r\n
				  header_end = cur - rb.data();
				  break;
				}
			  } else cur++;
			}
			// Read more data from the socket if the headers are not complete.
			if (!complete_header && 0 == rb.read_more(fiber))
			  return;
		  }
		  // Header is complete. Process it.
		  // Run the handler.
		  assert(rb.cursor <= rb.end);
		  ctx.body_start = std::string_view(rb.data() + header_end, rb.end - header_end);
		  ctx.prepare_request();

		  Req rq{ ctx };
		  Res resp(ctx);
		  try {
			api._call(c2m(ctx.method().c_str()), ctx.url(), rq, resp);
		  } catch (const http_error& e) {
			ctx.set_status(e.i());
			ctx.respond(e.what());
		  } catch (const std::runtime_error& e) {
			std::cerr << "INTERNAL SERVER ERROR: " << e.what() << std::endl;
			ctx.set_status(500);
			ctx.respond("Internal server error.");
		  }
		  ctx.respond_if_needed();

		  assert(rb.cursor <= rb.end);
		  // Update the cursor the beginning of the next request.
		  ctx.prepare_next_request();
		  // if read buffer is empty, we can flush the output buffer.
		  if (rb.empty())
			ctx.flush_responses();
		}
	  } catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	  }
	};
	auto date_thread = std::make_shared<std::thread>([]() {
	  fc::http_top_header.tick(); std::this_thread::sleep_for(std::chrono::seconds(1));
	});
	auto server_thread = std::make_shared<std::thread>([=] {
	  std::cout << "Starting FabCc::server on port " << port << std::endl;
	  // if constexpr (has_key<decltype(options)>(s::ssl_key)) {
		 //static_assert(has_key<decltype(options)>(s::ssl_certificate),
		 //			  "You need to provide both the ssl_certificate option and the ssl_key option.");
		 //std::string ssl_key = options.ssl_key;
		 //std::string ssl_cert = options.ssl_certificate;
		 //std::string ssl_ciphers = get_or(options, s::ssl_ciphers, "");
		 //start_tcp_server(ip, port, SOCK_STREAM, nthreads,
		 //				 fc::make_http_processor(std::move(handler)), ssl_key, ssl_cert, ssl_ciphers);
	  // } else {
	  // }
	  start_tcp_server(ip, port, SOCK_STREAM, nthreads, std::move(make_http_processor));//SOCK_DGRAM
      date_thread->join();
	});
	server_thread->join();
  }
} // namespace fc
