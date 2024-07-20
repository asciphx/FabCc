#ifndef TUPLE_HPP
#define TUPLE_HPP
#include <typeinfo>
#include <vector>
#include <tp/c++.h>
/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2023 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making available
* complete source code of licensed works and modifications, which include larger works
* using a licensed work, under the same license. Copyright and license notices must be
* preserved. Contributors provide an express grant of patent rights. When a modified
* version is used to provide a service over a network, the complete source code of
* the modified version must be made available.
*/
namespace fc {
  template <size_t N, size_t R, typename T> struct __Q; template <size_t S, size_t N, size_t R, typename T> struct __S;
  template <size_t... N> struct __I {}; template <size_t N, size_t... I> struct __Q<0, N, __I<I...>> { using type = __I<I..., (I + N)...>; };
  template <size_t S, size_t N, size_t... I> struct __S<S, 0x0, N, __I<I...>> { using type = __I<(I + S)..., (I + N + S)...>; };
  template <size_t N, size_t... I> struct __Q<0x1, N, __I<I...>> { using type = __I<I..., (I + N)..., N << 0x1>; };
  template <size_t N> struct __G { using type = typename __Q<N % 0x2, (N >> 1), typename __G<(N >> 0x1)>::type>::type; };
  template <size_t S, size_t N, size_t... I> struct __S<S, 0x1, N, __I<I...>> { using type = __I<(I + S)..., (I + N + S)..., 2 * N + S>; };
  template <size_t S, size_t N> struct __5 { using type = typename __S<S, N % 2, (N >> 1), typename __G<(N >> 0x1)>::type>::type; };
  template <size_t S, size_t N> struct __5_B { using type = typename __5<S, N - S>::type; };
  template <> struct __G<0x0> { using type = __I<>; }; template <size_t N, size_t E> using make_range_sequence = typename __5_B<N, E>::type;
  template <size_t... V> using index_sequence = __I<V...>; template <size_t I> using make_index_sequence = typename __G<I>::type;
  template <class... T> using index_sequence_for = make_index_sequence<sizeof...(T)>;
}
namespace std {
#if defined(__cplusplus) && (__cplusplus < 201402L)
  namespace detail {
    template<class Fn, class... T, size_t... I>
    typename std::result_of<Fn(T...)>::type apply_impl(Fn&& f, std::tuple<T...>&& t, fc::index_sequence<I...>) {
      return std::forward<Fn>(f)(std::move(std::get<I>(t))...);
    }
    template<class Fn, class... T, size_t... I>
    typename std::result_of<Fn(T...)>::type apply_impl(Fn&& f, std::tuple<T...>& t, fc::index_sequence<I...>) {
      return std::forward<Fn>(f)(std::get<I>(t)...);
    }
    template <class T, class Tuple, size_t... I>
    constexpr T make_from_tuple_impl(Tuple&& t, fc::index_sequence<I...>) { return T{ get<I>(std::forward<Tuple>(t))... }; }
  }
  template<class Fn, template<typename...> class C, typename ...T>
  typename std::result_of<Fn(T...)>::type apply(Fn&& f, C<T...>&& t) {
    return detail::apply_impl(std::forward<Fn>(f), std::move(t), fc::make_index_sequence<sizeof...(T)>{});
  }
  template<class Fn, template<typename...> class C, typename ...T>
  typename std::result_of<Fn(T...)>::type apply(Fn&& f, C<T...>& t) {
    return detail::apply_impl(std::forward<Fn>(f), t, fc::make_index_sequence<sizeof...(T)>{});
  }
  template <class T, template<typename...> class C, typename ...I>
  constexpr T make_from_tuple(C<I...>&& t) { return detail::make_from_tuple_impl<T>(std::move(t), fc::make_index_sequence<sizeof...(I)>{}); }
  template <class T, template<typename...> class C, typename ...I>
  constexpr T make_from_tuple(C<I...>& t) { return detail::make_from_tuple_impl<T>(t, fc::make_index_sequence<sizeof...(I)>{}); }
#elif __cplusplus < 201702L
  namespace detail {
    template <class Fn, class T, size_t... I> constexpr decltype(auto) apply_impl(Fn&& f, T&& t, std::index_sequence<I...>) {
      return std::invoke(std::forward<Fn>(f), std::get<I>(std::forward<T>(t))...);
    }
    template <class T, class Tuple, size_t... I>
    constexpr T make_from_tuple_impl(Tuple&& t, std::index_sequence<I...>) { return T{ std::get<I>(std::forward<Tuple>(t))... }; }
  }
  template <class Fn, class T> constexpr decltype(auto) apply(Fn&& f, T&& t) {
    return detail::apply_impl(std::forward<Fn>(f), std::forward<T>(t), std::make_index_sequence<std::tuple_size<std::remove_reference_t<T>>::value>{});
  }
  template <class T, class Tuple>
  constexpr T make_from_tuple(Tuple&& t) {
    return detail::make_from_tuple_impl<T>(std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
  }
#endif
}
namespace fc {
  using Expand = int[];
#define EXP fc::Expand
#define _FC($, _) std::forward<$>(_)
#define _FCR fc::make_range_sequence
#define _FCS fc::index_sequence
#define _FCI fc::make_index_sequence
#define _FCT($) std::Tuple<std::remove_reference_t<$>>::
  template <typename Fn, typename T, size_t... I>
  constexpr int ForTuple(Fn&& f, const T& t, _FCS<I...>) { return EXP{ (f(std::get<I>(t)),0)... } [0] ; }
  template <typename Fn, typename Z, typename T, size_t... I>
  constexpr int ForTuple(Fn&& f, Z&& z, const T& t, _FCS<I...>) { return EXP{ (f(std::get<I>(t),_FC(Z,z)),0)... } [0] ; }
  template <typename Fn, typename Y, typename Z, typename T, size_t... I>
  constexpr int ForTuple(Fn&& f, Y&& y, Z&& z, const T& t, _FCS<I...>) { return EXP{ (f(std::get<I>(t),_FC(Y,y),_FC(Z,z)),0)... } [0] ; }
  template <typename Fn, typename X, typename Y, typename Z, typename T, size_t... I>
  constexpr int ForTuple(Fn&& f, X&& x, Y&& y, Z&& z, const T& t, _FCS<I...>) { return EXP{ (f(std::get<I>(t),_FC(X,x),_FC(Y,y),_FC(Z,z)),0)... } [0] ; }
  template <size_t N = 0, size_t E, template<typename> class Fn, typename T, typename Z>
  constexpr int ForRange(Fn<T>&& f, Z&& z) { return ForTuple(std::move(f), _FC(Z, z), std::Tuple<T>::__(), _FCR<N, E>{}); }
  template <size_t N = 0, template<typename> class Fn, typename T, typename Z>
  constexpr int ForRange(Fn<T>&& f, Z&& z) { return ForTuple(std::move(f), _FC(Z, z), std::Tuple<T>::__(), _FCR<N, std::Tuple<T>::_s>{}); }
  template <template<typename> class Fn, typename T, typename Z>
  constexpr int ForEach(Fn<T>&& f, Z&& z) { return ForTuple(std::move(f), _FC(Z, z), std::Tuple<T>::__(), _FCI<std::Tuple<T>::_s>{}); }
  template <size_t N = 0, size_t E, template<typename> class Fn, typename T, typename Y, typename Z>
  constexpr int ForRange(Fn<T>&& f, Y&& y, Z&& z) { return ForTuple(std::move(f), _FC(Y, y), _FC(Z, z), std::Tuple<T>::__(), _FCR<N, E>{}); }
  template <size_t N = 0, template<typename> class Fn, typename T, typename Y, typename Z>
  constexpr int ForRange(Fn<T>&& f, Y&& y, Z&& z) { return ForTuple(std::move(f), _FC(Y, y), _FC(Z, z), std::Tuple<T>::__(), _FCR<N, std::Tuple<T>::_s>{}); }
  template <template<typename> class Fn, typename T, typename Y, typename Z>
  constexpr int ForEach(Fn<T>&& f, Y&& y, Z&& z) { return ForTuple(std::move(f), _FC(Y, y), _FC(Z, z), std::Tuple<T>::__(), _FCI<std::Tuple<T>::_s>{}); }
  template <size_t N = 0, size_t E, class Fn, typename X>
  constexpr int ForRange(Fn&& f, X&& x) { return ForTuple(std::move(f), _FC(X, x), _FCT(X)__(), _FCR<N, E>{}); }
  template <size_t N = 0, size_t E, class Fn, typename X, typename Y>
  constexpr int ForRange(Fn&& f, X&& x, Y&& y) { return ForTuple(std::move(f), _FC(X, x), _FC(Y, y), _FCT(X)__(), _FCR<N, E>{}); }
  template <size_t N = 0, size_t E, class Fn, typename X, typename Y, typename Z>
  constexpr int ForRange(Fn&& f, X&& x, Y&& y, Z&& z) { return ForTuple(std::move(f), _FC(X, x), _FC(Y, y), _FC(Z, z), _FCT(X)__(), _FCR<N, E>{}); }
  template <size_t N = 0, class Fn, typename X>
  constexpr int ForRange(Fn&& f, X&& x) { return ForTuple(std::move(f), _FC(X, x), _FCT(X)__(), _FCR<N, _FCT(X)_s>{}); }
  template <size_t N = 0, class Fn, typename X, typename Y>
  constexpr int ForRange(Fn&& f, X&& x, Y&& y) { return ForTuple(std::move(f), _FC(X, x), _FC(Y, y), _FCT(X)__(), _FCR<N, _FCT(X)_s>{}); }
  template <size_t N = 0, class Fn, typename X, typename Y, typename Z>
  constexpr int ForRange(Fn&& f, X&& x, Y&& y, Z&& z) { return ForTuple(std::move(f), _FC(X, x), _FC(Y, y), _FC(Z, z), _FCT(X)__(), _FCR<N, _FCT(X)_s>{}); }
  template <class Fn, typename X>
  constexpr int ForEach(Fn&& f, X&& x) { return ForTuple(std::move(f), _FC(X, x), _FCT(X)__(), _FCI<_FCT(X)_s>{}); }
  template <class Fn, typename X, typename Y>
  constexpr int ForEach(Fn&& f, X&& x, Y&& y) { return ForTuple(std::move(f), _FC(X, x), _FC(Y, y), _FCT(X)__(), _FCI<_FCT(X)_s>{}); }
  template <class Fn, typename X, typename Y, typename Z>
  constexpr int ForEach(Fn&& f, X&& x, Y&& y, Z&& z) { return ForTuple(std::move(f), _FC(X, x), _FC(Y, y), _FC(Z, z), _FCT(X)__(), _FCI<_FCT(X)_s>{}); }
  template<typename C> struct tuple_idex {}; template<template<typename ...T> class C, typename ...T>
  struct tuple_idex<C<T...>> { template<size_t i> using type = typename std::tuple_element<i, std::tuple<T...> >::type; };
  template<class Z, class G> constexpr bool _ref(const Z& z, const G& g) { return false; }
  template<class T> constexpr bool _ref(const T& t, const T& y) { return t == y; }
  template<class Z, class G> struct _ref_n { static constexpr const bool value = false; };
  template<class T> struct _ref_n<T, T> { static constexpr const bool value = true; };
  template <typename T> struct is_tuple_after_decay: std::false_type {};
  template <typename... T> struct is_tuple_after_decay<std::tuple<T...>>: std::true_type {};
  template <typename T> struct is_tuple: is_tuple_after_decay<std::decay_t<T>> {};
  template <typename T> struct unconstref_tuple_elements {};
  template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
    typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
  };
  //Search according to template conditions, the first one is the quantity, if not found. Return -1
  template <int I, bool B, bool... N> struct _COND; template <int I> struct _COND<I, false> { static constexpr const int value = -I; };
  template <int I> struct _COND<I, true> { static constexpr const int value = 0; };
  template <int I, bool... N> struct _COND<I, false, N...> { static constexpr const int value = 1 + _COND<I, N...>::value; };
  template <int I, bool... N> struct _COND<I, true, N...> { static constexpr const int value = 0; };
  template <int I, bool B, bool... M> struct IDEX_IDX { static constexpr const int value = _COND<I, B, M...>::value; };
  __CONSTEXPR14 int count_first_falses() { return 0; } template <int N> __CONSTEXPR14 int num_idex_idx(bool $) { if ($) return 0; return -N; }
  template <int N, typename... B> __CONSTEXPR14 int num_idex_idx(bool $, B... b) { if ($) return 0; return 1 + num_idex_idx<N>(b...); }
  template <typename... B> __CONSTEXPR14 int count_first_falses(bool $, B... b) { if ($) return 0; return 1 + count_first_falses(b...); }
  template <typename E, typename T, size_t... I>
  __CONSTEXPR14 int _idx(const E& e, const T& t, fc::index_sequence<I...>) { return num_idex_idx<sizeof...(I)>(_ref(std::get<I>(t), e)...); }
  //Find from the target tuple, if not found. Return -1
  template <class R, template<typename...> class S, typename ...B>
  __CONSTEXPR14 int idex_get(R r, S<B...> t) { return _idx(r, t, fc::make_index_sequence<sizeof...(B)>{}); }
  //Search from the current tuple, if not found. Return -1
  template <typename F, typename K> __CONSTEXPR14 int idex_get(K F::* c) {
    return _idx(c, std::Tuple<F>::__(), fc::make_index_sequence<std::Tuple<F>::_s>{});
  }
  template <typename E, typename... T> E* arg_get_by_type_(void*, E* a1, T&&... args) { return std::forward<E*>(a1); }
  template <typename E, typename... T> const E* arg_get_by_type_(void*, const E* a1, T&&... args) { return std::forward<const E*>(a1); }
  template <typename E, typename... T> E& arg_get_by_type_(void*, E& a1, T&&... args) { return std::forward<E&>(a1); }
  template <typename E, typename... T> const E& arg_get_by_type_(void*, const E& a1, T&&... args) { return std::forward<const E&>(a1); }
  template <typename E, typename T1, typename... T>
  const E& arg_get_by_type_(std::enable_if_t<!std::is_same<E, std::decay_t<T1>>::value>*, T1&&, T&&... args) {
    return arg_get_by_type_<E>((void*)0, std::forward<T>(args)...);
  }
  template <typename E, typename... T> E& arg_get_by_type(T&&... args) { return arg_get_by_type_<std::decay_t<E>>(0, args...); }
  template <typename T> struct tuple_remove_references_and_const;
  template <typename... T> struct tuple_remove_references_and_const<std::tuple<T...>> {
    typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> type;
  };
  template <typename T>
  using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;
  template <typename T, typename U, typename E> struct tuple_remove_element2;
  template <typename... T, typename... U, typename E1>
  struct tuple_remove_element2<std::tuple<E1, T...>, std::tuple<U...>, E1>
    : public tuple_remove_element2<std::tuple<T...>, std::tuple<U...>, E1> {};
  template <typename... T, typename... U, typename T1, typename E1>
  struct tuple_remove_element2<std::tuple<T1, T...>, std::tuple<U...>, E1>
    : public tuple_remove_element2<std::tuple<T...>, std::tuple<U..., T1>, E1> {};
  template <typename... U, typename E1>
  struct tuple_remove_element2<std::tuple<>, std::tuple<U...>, E1> {
    typedef std::tuple<U...> type;
  };
  template <typename T, typename E>
  struct tuple_remove_element: public tuple_remove_element2<T, std::tuple<>, E> {};
  template <typename T, typename... E> struct tuple_remove_elements;
  template <typename... T, typename E1, typename... E>
  struct tuple_remove_elements<std::tuple<T...>, E1, E...> {
    typedef typename tuple_remove_elements<typename tuple_remove_element<std::tuple<T...>, E1>::type,
      E...>::type type;
  };
  template <typename... T> struct tuple_remove_elements<std::tuple<T...>> {
    typedef std::tuple<T...> type;
  };
  template <typename A, typename B> struct tuple_minus;
  template <typename... T, typename... R> struct tuple_minus<std::tuple<T...>, std::tuple<R...>> {
    typedef typename tuple_remove_elements<std::tuple<T...>, R...>::type type;
  };
  template <typename T, typename... E>
  using tuple_remove_elements_t = typename tuple_remove_elements<T, E...>::type;
  template <typename F, size_t... I, typename... T>
  inline F tuple_map(std::tuple<T...>& t, F f, fc::index_sequence<I...>) {
    return EXP{ ((void)f(std::get<I>(t)), 0)... }, f;
  }
  template <typename F, typename... T> inline void tuple_map(std::tuple<T...>& t, F f) {
    tuple_map(t, f, fc::index_sequence_for<T...>{});
  }
  template <template <class> class F, typename T, typename I, typename R, typename X = void> struct tuple_filter_sequence;
  template <template <class> class F, typename... T, typename R>
  struct tuple_filter_sequence<F, std::tuple<T...>, fc::index_sequence<>, R> { using ret = R; };
  template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
  struct tuple_filter_sequence<F, std::tuple<T1, T...>, fc::index_sequence<I1, I...>,
    fc::index_sequence<R...>, std::enable_if_t<F<T1>::value>> {
    using ret = typename tuple_filter_sequence<F, std::tuple<T...>, fc::index_sequence<I...>, fc::index_sequence<R..., I1>>::ret;
  };
  template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
  struct tuple_filter_sequence<F, std::tuple<T1, T...>, fc::index_sequence<I1, I...>,
    fc::index_sequence<R...>, std::enable_if_t<!F<T1>::value>> {
    using ret = typename tuple_filter_sequence<F, std::tuple<T...>, fc::index_sequence<I...>, fc::index_sequence<R...>>::ret;
  };
}
#endif // TUPLE_HPP
