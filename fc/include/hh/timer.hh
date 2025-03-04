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
#include "hpp/rb_tree.hpp"
#include <chrono>
namespace fc {
  // Timer implementation using RBTree
  struct Timer {
    struct Node {
      Node(const Node& _):time(_.time), id(_.id) {}
      Node(std::chrono::steady_clock::time_point&& t, uint64_t& i):time(std::move(t)), id(i) {}
      Node(const std::chrono::steady_clock::time_point& t, uint64_t& i):time(t), id(i) {}
      Node():id(0) {}
      void operator = (Node&& _) { time = std::move(_.time); id = std::move(_.id); }
      std::chrono::steady_clock::time_point time;
      uint64_t id;
      bool operator<(const Node&) const;
      explicit operator bool() const noexcept { return this->time.time_since_epoch().count(); }
      bool operator!() const noexcept { return this->time.time_since_epoch().count() == 0; }
    };
    virtual Node add_ms(unsigned int ms, std::function<void()>&& cb);
    virtual Node add_s(unsigned int s, std::function<void()>&& cb);
    virtual void tick();
    virtual int64_t time_to_next() const;
    void cancel(Node& id);//If Node.time.time_since_epoch().count() is 0 it will be skipped
  protected:
    RBTree<Node, std::function<void()>> timers;
    uint64_t next_id = 0;
    using ______ = RBTree<Node, std::function<void()>>::Nod;
  };
}
#endif
