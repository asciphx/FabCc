#include "hh/spinlock.hh"
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
#ifdef _WIN32
#include "h/windows.h"
#endif // _WIN32
#include <thread>
namespace fc {
  void spinlock::lock() noexcept {
    // Optimistically try to acquire the lock
    while (lock_.exchange(true, std::memory_order_acquire)) {
      // Spin wait until the lock is potentially free
      // Wait for lock to be released without generating cache misses
      while (lock_.load(std::memory_order_relaxed)) {
        // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
        // hyper-threads
#if defined(_WIN32)
        YieldProcessor(); // Windows (x86/x64)
#elif defined(__x86_64__) || defined(__i386__)
        __builtin_ia32_pause(); // GCC/Clang x86/x64
#elif defined(__arm__) || defined(__aarch64__)
        __asm__ __volatile__("yield" ::: "memory"); // ARM/ARM64
#elif defined(__riscv)
        __asm__ __volatile__("nop;nop\nnop" ::: "memory");// RISC-V
#else
        // Fallback
        std::this_thread::yield();
#endif
      }
    }
  }
  bool spinlock::try_lock() noexcept {
    // First do a relaxed load to check if lock is free in order to prevent
    // unnecessary cache misses if someone does while(!try_lock())
    return !lock_.load(std::memory_order_relaxed) && !lock_.exchange(true, std::memory_order_acquire);
  }
  void spinlock::unlock() noexcept {
    lock_.store(false, std::memory_order_release);
  }
}