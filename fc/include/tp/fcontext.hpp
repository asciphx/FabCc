//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#ifndef FCONTEXT_HH
#define FCONTEXT_HH
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
#undef _CONTEXT_CALLDECL
#if defined(i386) || defined(__i386__) || defined(__i386) \
     || defined(__i486__) || defined(__i586__) || defined(__i686__) \
     || defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) \
     || defined(__I86__) || defined(__INTEL__) || defined(__IA32__) \
     || defined(_M_IX86) || defined(_I86_)
# define _CONTEXT_CALLDECL __cdecl
#else
# define _CONTEXT_CALLDECL
#endif
#include <stddef.h>
  typedef void* fcontext_t;
  typedef struct {
    fcontext_t  fctx;
    void* data;
  } transfer_t;
  extern "C" transfer_t _CONTEXT_CALLDECL jump_fcontext(fcontext_t const to, void* vp);
  extern "C" fcontext_t _CONTEXT_CALLDECL make_fcontext(void* sp, size_t size, void (*fn)(transfer_t));
  // based on an idea of Giovanni Derreta
  extern "C" transfer_t _CONTEXT_CALLDECL ontop_fcontext(fcontext_t const to, void* vp, transfer_t(*fn)(transfer_t));
  // This C++ tail of ontop_fcontext() allocates transfer_t{ from, vp }
  // on the stack.  If fn() throws a C++ exception, then the C++ runtime
  // must remove this tail's stack frame.
  extern "C" inline transfer_t _CONTEXT_CALLDECL ontop_fcontext_tail(void* vp, transfer_t(*fn)(transfer_t), fcontext_t const from) {
    return fn(transfer_t{ from, vp });
  }
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
#endif
