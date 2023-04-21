#ifndef CPP_HH
#define CPP_HH
#include <tuple>
#include <functional>
#include <type_traits>
#include <utility>
#if (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
#define unlikely(x) __builtin_expect((x), 0)
#define likely(x) __builtin_expect(!!(x), 1)
#else
#define unlikely(x) x
#define likely(x) x
#endif
#if defined(_MSVC_LANG)
#define ALIGN(s) __declspec(align(s))
#define FORCE_INLINE __forceinline
#define NEVER_INLINE __declspec(noinline)
#ifdef _WIN64
typedef signed __int64 ssize_t;
#elif _WIN32
typedef signed int ssize_t;
#endif
#else
#define ALIGN(s) __attribute__((aligned(s)))
#define FORCE_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE inline __attribute__((noinline))
#endif
typedef float f32;
typedef double f64;
typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
namespace std {
#if (defined(_HAS_CXX17) && _HAS_CXX17 == 1) || (defined(__cplusplus) && __cplusplus >= 201703L)
  #define __INLINE inline
  #define __CONSTEXPR constexpr
  template <typename _Tp> inline constexpr size_t tuple_size_V = tuple_size<_Tp>::value;
#else//__cplusplus <= 201402L
  #define __INLINE
  #define __CONSTEXPR
  template <typename _Tp> constexpr size_t tuple_size_V = tuple_size<_Tp>::value;
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
  struct in_place_t { // tag used to select a constructor which initializes a contained object in place
	explicit in_place_t() = default;
  };
  constexpr in_place_t in_place{};
  template <class>
  struct in_place_type_t { // tag that selects a type to construct in place
	explicit in_place_type_t() = default;
  };
  template <class _Ty>
  constexpr in_place_type_t<_Ty> in_place_type{};
/// Used to represent an optional with no data; essentially a bool
  class monostate {};
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
#endif // !CPP_HH
