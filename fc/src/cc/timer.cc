#include "hh/timer.hh"
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
  bool Timer::Node::operator<(const Timer::Node& other) const {
    return time < other.time || (time == other.time && id < other.id);
  }
  Timer::Node Timer::add_s(unsigned int s, std::function<void()>&& cb) {
    Node node( std::chrono::steady_clock::now() + std::chrono::seconds(s), ++next_id);
    timers.insert(node, std::move(cb)); return node;
  }
  Timer::Node Timer::add_ms(unsigned int ms, std::function<void()>&& cb) {
    Node node{ std::chrono::steady_clock::now() + std::chrono::milliseconds(ms), ++next_id };
    timers.insert(node, std::move(cb)); return node;
  }
  void Timer::tick() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    while (!timers.empty()) {
      ______* node = timers.minimum(reinterpret_cast<______*&>(timers));
      if (node == nullptr || node->key.time > now) break;
      node->value(); timers.remove(node->key);
    }
  }
  int64_t Timer::time_to_next() const {
    if (timers.empty()) return -1;
    ______* node = timers.minimum(reinterpret_cast<______*&>(const_cast<RBTree<Node, std::function<void()>>&>(timers)));
    if (node == nullptr) return -1;
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
      node->key.time - std::chrono::steady_clock::now()).count();
    return diff > 0 ? diff : 0;
  }
  void Timer::cancel(Node& id) {//Prevent duplicate deletion
    if(id.time.time_since_epoch().count()) {
      ______* node = timers.find(id); id.time = std::chrono::steady_clock::time_point();
      if (node != nullptr) {
        timers.remove(node->key);
      }
    }
  }
}