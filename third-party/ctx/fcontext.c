 #ifdef _WIN32
#if defined _MSC_VER
#if defined(_M_X64)
#  pragma pack(push,16)
#else
#  pragma pack(push,8)
#endif
#endif
#else
#if defined __CODEGEARC__
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#elif defined __BORLANDC__
#if __BORLANDC__ != 0x600
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#endif
#endif
#endif
#include "fcontext.h"
transfer_t _CONTEXT_CALLDECL ontop_fcontext_tail(void* vp, transfer_t(*fn)(transfer_t), fcontext_t const from) {
  transfer_t t; t.fctx = from; t.data = vp; return fn(t);// return fn((transfer_t){ from, vp });
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