#ifndef SPINLOCK_HH
#define SPINLOCK_HH
#include <atomic>
//from https://rigtorp.se/spinlock
namespace fc {
  struct spinlock {
    std::atomic<bool> lock_ = { 0 };
  public:
    void lock() noexcept;
    bool try_lock() noexcept;
    void unlock() noexcept;
  };
}
#endif
