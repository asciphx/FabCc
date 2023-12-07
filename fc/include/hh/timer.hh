#ifndef TIMER_HH
#define TIMER_HH
#include <queue>
#include <chrono>
#include <thread>
#include <functional>
#include <unordered_map>
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
#if defined _WIN32
#include <basetsd.h>
#define _sock_type UINT_PTR
#else
#define _sock_type int
#endif
namespace fc {
  struct ______ {
    std::chrono::time_point<std::chrono::system_clock> t; std::function<void()> f;
    bool operator<(const ______& l) const { return l.t < this->t; }
  };
  class Timer {
  protected:
    std::priority_queue<______, std::vector<______>> min_heap;
  public:
    virtual void add_s(unsigned int seconds, std::function<void()>&& func);
    virtual void add_ms(unsigned int milliseconds, std::function<void()>&& func);
    virtual void tick();
  };
}
#endif
