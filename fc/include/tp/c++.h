#ifndef CPP_HH
#define CPP_HH
#include <tuple>
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
