#ifndef HTTP_TOP_HPP
#define HTTP_TOP_HPP
#include <time.h>
#include <chrono>
#include "hpp/string_view.hpp"
#include "hh/timer.hh"
#if defined(_MSC_VER)
#define _NOW_VER RES_TP
#else
#define _NOW_VER std::chrono::steady_clock::now()
#endif
namespace fc {
  static struct tm* RES_NOW;
  static int64_t RES_TIME_T;
  static std::chrono::steady_clock::time_point RES_TP = std::chrono::steady_clock::now();
  static _FORCE_INLINE tm now() { return *RES_NOW; }
  static _FORCE_INLINE int64_t nowStamp(short& i) { return RES_TIME_T + i; }
  static _FORCE_INLINE int64_t nowStamp(short&& i) { return RES_TIME_T + std::move(i); }
  static _FORCE_INLINE int64_t nowStamp() { return RES_TIME_T; }
  struct timer: public Timer {
    inline Node add(std::function<void()>&& cb) {
      Node node{ _NOW_VER, ++next_id }; timers.insert(node, std::move(cb)); return node;
    }
    inline Node add_s(unsigned int s, std::function<void()>&& cb) {
      Node node{ _NOW_VER + std::chrono::seconds(s), ++next_id };
      timers.insert(node, std::move(cb)); return node;
    }
    inline Node add_ms(unsigned int ms, std::function<void()>&& cb) {
      Node node{ _NOW_VER + std::chrono::milliseconds(ms), ++next_id };
      timers.insert(node, std::move(cb)); return node;
    }
    inline void tick() {
      while (!timers.empty()) {
        ______* node = timers.minimum(reinterpret_cast<______*&>(timers));
        if (node == nullptr || node->key.time > _NOW_VER) break;
        node->value(); timers.remove(node->key);
      }
    }
  };
  struct double_buffer {
    double_buffer() { this->p1 = this->b1; this->p2 = this->b2; }
    char* p1;
    char* p2;
    char b1[0X20];
    char b2[0x20];
  };
  struct http_top_header_builder {
    inline std::string_view top_header() { return std::string_view(tmp.p1, top_header_size); };
    _FORCE_INLINE void tick() {
      RES_TP = std::chrono::steady_clock::now();
#if defined(_MSC_VER)
      if (++i == 60) {
        i = 0; time(&RES_TIME_T); localtime_s(RES_NOW, &RES_TIME_T);
        top_header_size = int(strftime(tmp.p2, 32, "%a, %d %b %Y %T", RES_NOW)); std::swap(tmp.p1, tmp.p2);
      }
#elif defined(__MINGW32__)
      if (++i == 60) { i = 0; time(&RES_TIME_T); localtime_s(RES_NOW, &RES_TIME_T); }
#else
      if (++i == 250) {
        i = 0; time(&RES_TIME_T); localtime_r(&RES_TIME_T, RES_NOW);
        top_header_size = int(strftime(tmp.p2, 32, "%a, %d %b %Y %T", RES_NOW)); std::swap(tmp.p1, tmp.p2);
      }
#endif
    }
    fc::double_buffer tmp;
    int i = 9;
    int top_header_size;
  };
} // namespace fc
#endif
