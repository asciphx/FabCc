#include "fixedsize_stack.hh"
#ifdef _WIN32
extern "C" {
#include <windows.h>
}
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <assert.h>
// x86_64
// test x86_64 before i386 because icc might
// define __i686__ for x86_64 too
#if defined(__x86_64__) || defined(__x86_64) \
    || defined(__amd64__) || defined(__amd64) \
    || defined(_M_X64) || defined(_M_AMD64)
// Windows seams not to provide a constant or function
// telling the minimal stacksize
# define MIN_STACKSIZE  8 * 1024
#else
# define MIN_STACKSIZE  4 * 1024
#endif
namespace {
  size_t pagesize() noexcept {
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	return static_cast<size_t>(si.dwPageSize);
  }
}
namespace context {
  // Windows seams not to provide a limit for the stacksize
  // libcoco uses 32k+4k bytes as minimum
  bool stack_traits::is_unbounded() noexcept {
	return true;
  }
  size_t stack_traits::page_size() noexcept {
	static size_t size = pagesize();
	return size;
  }
  size_t stack_traits::default_size() noexcept {
	return 128 * 1024;
  }
  // because Windows seams not to provide a limit for minimum stacksize
  size_t stack_traits::minimum_size() noexcept {
	return MIN_STACKSIZE;
  }
  // because Windows seams not to provide a limit for maximum stacksize
  // maximum_size() can never be called (pre-condition ! is_unbounded() )
  size_t stack_traits::maximum_size() noexcept {
	assert(!is_unbounded());
	return  1 * 1024 * 1024 * 1024; // 1GB
  }
}
#else
extern "C" {
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
}
#include <algorithm>
#include <cmath>
#include <assert.h>
namespace {
  size_t pagesize() noexcept {
	// conform to POSIX.1-2001
	return ::sysconf(_SC_PAGESIZE);
  }
  rlim_t stacksize_limit_() noexcept {
	rlimit limit;
	// conforming to POSIX.1-2001
	::getrlimit(RLIMIT_STACK, &limit);
	return limit.rlim_max;
  }
  rlim_t stacksize_limit() noexcept {
	static rlim_t limit = stacksize_limit_();
	return limit;
  }
}
namespace context {
  bool
	stack_traits::is_unbounded() noexcept {
	return RLIM_INFINITY == stacksize_limit();
  }
  size_t
	stack_traits::page_size() noexcept {
	static size_t size = pagesize();
	return size;
  }
  size_t
	stack_traits::default_size() noexcept {
	return 128 * 1024;
  }
  size_t
	stack_traits::minimum_size() noexcept {
	return MINSIGSTKSZ;
  }
  size_t
	stack_traits::maximum_size() noexcept {
	assert(!is_unbounded());
	return static_cast<size_t>(stacksize_limit());
  }
}
#endif
