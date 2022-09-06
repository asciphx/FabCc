//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#include <cstddef>
#include <cstdlib>
#include <new>
#include <new>
#include <assert.h>
namespace context {
#if __cplusplus >= 201103L
  struct stack_context {
	size_t             size{ 0 };
	void* sp{ nullptr };
  };
#else
  struct stack_context {
	size_t             size;
	void* sp;
	stack_context():
	  size(0),
	  sp(0) {}
  };
#endif
  struct stack_traits {
	static bool is_unbounded() noexcept;
	static size_t page_size() noexcept;
	static size_t default_size() noexcept;
	static size_t minimum_size() noexcept;
	static size_t maximum_size() noexcept;
  };
  template< typename traitsT >
  class basic_fixedsize_stack {
  private:
	size_t     size_;
  public:
	typedef traitsT traits_type;
	basic_fixedsize_stack(size_t size = traits_type::default_size()) noexcept:
	  size_(size) {}
	stack_context allocate() {
	  void* vp = std::malloc(size_);
	  if (!vp) {
		throw std::bad_alloc();
	  }
	  stack_context sctx;
	  sctx.size = size_;
	  sctx.sp = static_cast<char*>(vp) + sctx.size;
	  return sctx;
	}
	void deallocate(stack_context& sctx) noexcept {
	  assert(sctx.sp);
	  void* vp = static_cast<char*>(sctx.sp) - sctx.size;
	  std::free(vp);
	}
  };
  typedef basic_fixedsize_stack< stack_traits >  fixedsize_stack;
  typedef fixedsize_stack default_stack;
}