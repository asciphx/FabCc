//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#ifndef CONTEXT_FIXEDSIZE_H
#define CONTEXT_FIXEDSIZE_H
#include <cstddef>
#include <cstdlib>
#include <new>
#include <assert.h>
#if defined _MSC_VER
#if defined(_M_X64)
#  pragma pack(push,16)
#else
#  pragma pack(push,8)
#endif
#elif defined __CODEGEARC__
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#elif defined __BORLANDC__
#if __BORLANDC__ != 0x600
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#endif
#endif
namespace ctx {
#if __cplusplus >= 201103L
  struct stack_context { size_t size{ 0 }; void* sp{ nullptr }; };
#else
  struct stack_context { size_t size; void* sp; stack_context(): size(0), sp(0) {} };
#endif
  struct stack_traits {
	static bool is_unbounded() noexcept; static size_t page_size() noexcept; static size_t default_size() noexcept;
	static size_t minimum_size() noexcept; static size_t maximum_size() noexcept;
  };
  class fixedsize_stack {
	size_t size_;
  public:
	fixedsize_stack(size_t size = stack_traits::default_size()) noexcept: size_(size) {}
	stack_context allocate() {
	  void* vp = std::malloc(size_); if (!vp) { throw std::bad_alloc(); }
	  stack_context sctx; sctx.size = size_; sctx.sp = static_cast<char*>(vp) + size_; return sctx;
	}
	void deallocate(stack_context& sctx) noexcept {
	  assert(sctx.sp); void* vp = static_cast<char*>(sctx.sp) - sctx.size; std::free(vp);
	}
  };
}
#if defined _MSC_VER
#pragma pack(pop)
#elif defined __CODEGEARC__
#  pragma option pop
#pragma nopushoptwarn
#elif defined __BORLANDC__
#if __BORLANDC__ != 0x600
#  pragma option pop
#pragma nopushoptwarn
#endif
#endif
#endif // CONTEXT_FIXEDSIZE_H