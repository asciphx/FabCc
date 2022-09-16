#ifndef CPP_HH
#define CPP_HH
#include <functional>
#include <type_traits>
#include <utility>
#if (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
#define BOOST_UNLIKELY(x) __builtin_expect(x, 0)
#define BOOST_LIKELY(x) __builtin_expect(x, 1)
#else
#define BOOST_UNLIKELY(x) x
#define BOOST_LIKELY(x) x
#endif
#if defined _MSC_VER
#if defined(_M_X64)
#  pragma pack(push,16)
#else
#  pragma pack(push,8)
#endif
# pragma warning(push)
#elif defined __CODEGEARC__
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#elif defined __BORLANDC__
#if __BORLANDC__ != 0x600
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#endif
#endif
namespace std {
#if (defined(_HAS_CXX17) && _HAS_CXX17 == 1) || (defined(__cplusplus) && __cplusplus >= 201703L)
  #define __INLINE inline
  #define __CONSTEXPR constexpr
#else//__cplusplus <= 201402L
  #define __INLINE
  #define __CONSTEXPR
  template< typename Fn, typename ... Args >
  typename std::enable_if<
	std::is_member_pointer< typename std::decay< Fn >::type >::value,
	typename std::result_of< Fn && (Args && ...) >::type
  >::type
	invoke(Fn&& fn, Args && ... args) {
	return std::mem_fn(fn)(std::forward< Args >(args) ...);
  }
  template< typename Fn, typename ... Args >
  typename std::enable_if<
	!std::is_member_pointer< typename std::decay< Fn >::type >::value,
	typename std::result_of< Fn && (Args && ...) >::type
  >::type
	invoke(Fn&& fn, Args && ... args) {
	return std::forward< Fn >(fn)(std::forward< Args >(args) ...);
  }
#endif
// http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
  template< typename X, typename Y >
  using disable_overload =
    typename std::enable_if<
        ! std::is_base_of<
            X,
            typename std::decay< Y >::type
        >::value
    >::type;
}
#if defined _MSC_VER
# pragma warning(pop)
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
#endif // !CPP_HH
