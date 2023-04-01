#ifndef TIMER_HH
#define TIMER_HH
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <string>
/*
 * This software is licensed under the AGPL-3.0 License.
 *
 * Copyright (C) 2023 Asciphx
 *
 * Permissions of this strongest copyleft license are conditioned on making available
 * complete source code of licensed works and modifications, which include larger works
 * using a licensed work, under the same license. Copyright and license notices must be
 * preserved. Contributors provide an express grant of patent rights. When a modified
 * version is used to provide a service over a network, the complete source code of
 * the modified version must be made available.
 */
namespace fc {
  static struct tm* RES_NOW;
  static int64_t RES_TIME_T, RES_last;
  static std::string RES_DATE_STR;
  static inline tm now() { return *RES_NOW; }
  static inline int64_t nowStamp(short& i) { return RES_TIME_T + i; }
  static inline int64_t nowStamp(short&& i) { return RES_TIME_T + i; }
  static inline int64_t nowStamp() { return RES_TIME_T; }
  class Timer {
    std::atomic<bool> alive{ false };
  public:
    void setTimeout(std::function<void()>&& func, uint32_t milliseconds);
    void setInterval(std::function<void()>&& func, uint32_t milliseconds);
    void setTimeoutSec(std::function<void()>&& func, uint32_t seconds);
    void setIntervalSec(std::function<void()>&& func, uint32_t seconds);
    bool idle();
    void stop();
  };
}
#endif
