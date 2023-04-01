#include <hh/spinlock.hh>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winnt.h>
#endif // _WIN32
namespace fc {
  void spinlock::lock() noexcept {
    for (;;) {
      // Optimistically assume the lock is free on the first try
      if (!lock_.exchange(true, std::memory_order_acquire)) {
        break;
      }
      // Wait for lock to be released without generating cache misses
      while (lock_.load(std::memory_order_relaxed)) {
        // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
        // hyper-threads
#ifdef _WIN32
        YieldProcessor();
#else
        __builtin_ia32_pause();
#endif // _WIN32
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