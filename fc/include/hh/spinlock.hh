#ifndef SPINLOCK_HH
#define SPINLOCK_HH
#include <atomic>
//from https://rigtorp.se/spinlock
namespace fc {
/**
 * spinlock lock;
 * int counter = 0;
 * void increment() {
 * lock.lock();
 * counter++;
 * lock.unlock();
 * }
 *///Multiple threads update a shared counter.
  struct spinlock {
    std::atomic<bool> lock_ = { 0 };
  public:
    void lock() noexcept;
    bool try_lock() noexcept;
    void unlock() noexcept;
  };
}
#endif