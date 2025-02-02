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
  void Timer::add_s(unsigned int s, std::function<void()>&& func) {
    min_heap.push(______{ std::move(std::chrono::steady_clock::now() + std::chrono::seconds(s)) , std::move(func) });
  }
  void Timer::add_ms(unsigned int ms, std::function<void()>&& func) {
    min_heap.push(______{ std::move(std::chrono::steady_clock::now() + std::chrono::milliseconds(ms)) , std::move(func) });
  }
  void Timer::tick() {
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    while (!min_heap.empty()) { const ______& next = min_heap.top(); if (next.t <= now) next.f(), min_heap.pop(); else break; }
  }
}