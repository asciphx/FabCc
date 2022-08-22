#ifndef TUPLE_HPP
#define TUPLE_HPP
#include <tuple>
#include <typeinfo>
#include <type_traits>
#include <vector>
namespace fc {
  using Expand = int[];
#define Exp (void)fc::Expand
  template <typename T, typename Fn, std::size_t... I>
  inline constexpr void ForEachTuple(const T& tuple, Fn&& fn, std::index_sequence<I...>) { Exp{ ((void)fn(std::get<I>(tuple)), 0)... }; }
  template <typename T, typename Fn>
  inline constexpr void ForEachField(T* value, Fn&& fn) {
	ForEachTuple(T::Tuple, [value, &fn](auto field) { fn(value->*(field)); }, std::make_index_sequence<std::tuple_size_v<decltype(T::Tuple)>>{});
  }
  template <std::size_t I, typename T, typename Fn>
  inline constexpr void ForEachField(T* value, Fn&& fn) {
	ForEachTuple(T::Tuple, [value, &fn](auto field) { fn(value->*(field)); }, std::make_index_sequence<I>{});
  }
  
  template <typename T> constexpr inline auto Tuple() { return std::make_tuple(); }
  template<typename C> struct tuple_idex {};
  template<template<typename ...T> class C, typename ...T>
  struct tuple_idex<C<T...>> { template<size_t i> using type = typename std::tuple_element<i, std::tuple<T...> >::type; };
  template <class T> struct is_vector : std::false_type {};
  template <class T> struct is_vector<T[]> : std::false_type {};
  template <class T> struct is_vector<std::vector<T>> : std::true_type {};
  template<typename C> struct vector_pack {};
  template<template<typename, typename> class C, typename A, typename B> struct vector_pack<C<A, B>> { using type = A; };
  template<typename T> using vector_pack_t = typename vector_pack<T>::type;
  
  template<typename T, typename P> constexpr const P ExP(P T::* const) {}//ExP(K)Remove tuple of this &property
  template<typename T, typename P> constexpr const P T::* ExP(P) {}//ExP<T>(K)Set tuple of this &property
  template<typename T, typename K> constexpr T ExT(K T::* const) { return T(); }//ExT(K)Get tuple of this &key
  template <typename T> struct is_tuple_after_decay: std::false_type {};
  template <typename... T> struct is_tuple_after_decay<std::tuple<T...>>: std::true_type {};
  template <typename T> struct is_tuple: is_tuple_after_decay<std::decay_t<T>> {};
  template <typename T> struct unconstref_tuple_elements {};
  template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
	typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
  };
  constexpr int count_first_falses() { return 0; }
  template <typename... B> constexpr int count_first_falses(bool b1, B... b) {
	if (b1) return 0; else return 1 + count_first_falses(b...);
  }
  template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E* a1, T&&... args) {
	return std::forward<E*>(a1);
  }
  template <typename E, typename... T>
  decltype(auto) arg_get_by_type_(void*, const E* a1, T&&... args) {
	return std::forward<const E*>(a1);
  }
  template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E& a1, T&&... args) {
	return std::forward<E&>(a1);
  }
  template <typename E, typename... T>
  decltype(auto) arg_get_by_type_(void*, const E& a1, T&&... args) {
	return std::forward<const E&>(a1);
  }
  template <typename E, typename T1, typename... T>
  decltype(auto) arg_get_by_type_(std::enable_if_t<!std::is_same<E, std::decay_t<T1>>::value>*,
	T1&&, T&&... args) {
	return arg_get_by_type_<E>((void*)0, std::forward<T>(args)...);
  }
  template <typename E, typename... T> decltype(auto) arg_get_by_type(T&&... args) {
	return arg_get_by_type_<std::decay_t<E>>(0, args...);
  }
  template <typename E, typename... T> decltype(auto) type_get(std::tuple<T...>& tuple) {
	typedef std::decay_t<E> DE; return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
  }
  template <typename E, typename... T> decltype(auto) type_get(std::tuple<T...>&& tuple) {
	typedef std::decay_t<E> DE; return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
  }
  template <typename T, typename U> struct tuple_embeds: public std::false_type {};
  template <typename... T, typename U>
  struct tuple_embeds<std::tuple<T...>, U>
	: public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
	sizeof...(T)> {};
  template <typename U, typename... T> struct tuple_embeds_any_ref_of: public std::false_type {};
  template <typename U, typename... T>
  struct tuple_embeds_any_ref_of<std::tuple<T...>, U>
	: public tuple_embeds<std::tuple<std::decay_t<T>...>, std::decay_t<U>> {};
  template <typename T> struct tuple_remove_references;
  template <typename... T> struct tuple_remove_references<std::tuple<T...>> {
	typedef std::tuple<std::remove_reference_t<T>...> type;
  };
  template <typename T> using tuple_remove_references_t = typename tuple_remove_references<T>::type;
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
  inline F tuple_map(std::tuple<T...>& t, F f, std::index_sequence<I...>) {
	return (void)std::initializer_list<int>{((void)f(std::get<I>(t)), 0)...}, f;
  }
  template <typename F, typename... T> inline void tuple_map(std::tuple<T...>& t, F f) {
	tuple_map(t, f, std::index_sequence_for<T...>{});
  }
  template <typename F, size_t... I, typename T>
  inline decltype(auto) tuple_transform(T&& t, F f, std::index_sequence<I...>) {
	return std::make_tuple(f(std::get<I>(std::forward<T>(t)))...);
  }
  template <typename F, typename T> inline decltype(auto) tuple_transform(T&& t, F f) {
	return tuple_transform(std::forward<T>(t), f, std::make_index_sequence < std::tuple_size<std::decay_t<T>>{} > {});
  }
  template <template <class> class F, typename T, typename I, typename R, typename X = void>
  struct tuple_filter_sequence;
  template <template <class> class F, typename... T, typename R>
  struct tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<>, R> { using ret = R; };
  template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
  struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
	std::index_sequence<R...>, std::enable_if_t<F<T1>::value>> {
	using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
	  std::index_sequence<R..., I1>>::ret;
  };
  template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
  struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
	std::index_sequence<R...>, std::enable_if_t<!F<T1>::value>> {
	using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
	  std::index_sequence<R...>>::ret;
  };
  template <std::size_t... I, typename T>
  decltype(auto) Tuple(std::index_sequence<I...>, T&& t) { return std::make_tuple(std::get<I>(t)...); }
  template <template <class> class F, typename T> decltype(auto) tuple_filter(T&& t) {
	using seq = typename tuple_filter_sequence<
	  F, std::decay_t<T>, std::make_index_sequence<std::tuple_size<std::decay_t<T>>::value>, std::index_sequence<>>::ret;
	return Tuple(seq{}, t);
  }
  template <typename... E, typename F> constexpr void apply_each(F&& f, E&&... e) {
	(void)std::initializer_list<int>{((void)f(std::forward<E>(e)), 0)...};
  }
  template <typename... E, typename F, typename R>
  constexpr auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e) { return reduce(f(std::forward<E>(e))...); }
  template <typename T, typename F> constexpr void tuple_map(T&& t, F&& f) {
	return std::apply([&](auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); }, std::forward<T>(t));
  }
  template <typename T, typename F> constexpr auto tuple_reduce(T&& t, F&& f) {
	return std::apply(std::forward<F>(f), std::forward<T>(t));
  }
  template <typename T, typename F, typename R>
  decltype(auto) tuple_map_reduce(T&& m, F map, R reduce) {
	auto fun = [&](auto... e) { return tuple_map_reduce_impl(map, reduce, e...); }; return std::apply(fun, m);
  }
  template <typename F, typename... M, typename M1> constexpr auto Tuple(M1 m1, M... m) {
	if constexpr (std::is_same<M1, F>::value) return Tuple<F>(m...);
	else return std::tuple_cat(std::make_tuple(m1), Tuple<F>(m...));
  }
  template <typename F, typename... M> constexpr auto tuple_filter(const std::tuple<M...>& m) {
	auto fun = [](auto... e) { return Tuple<F>(e...); }; return std::apply(fun, m);
  }
  template <typename... T> struct typelist {};
  template <typename... T1, typename... T2>
  constexpr auto typelist_cat(typelist<T1...> t1, typelist<T2...> t2) { return typelist<T1..., T2...>(); }
  template <typename T> struct typelist_to_tuple {};
  template <typename... T> struct typelist_to_tuple<typelist<T...>> { typedef std::tuple<T...> type; };
  template <typename T> struct tuple_to_typelist {};
  template <typename... T> struct tuple_to_typelist<std::tuple<T...>> { typedef typelist<T...> type; };
  template <typename T> using typelist_to_tuple_t = typename typelist_to_tuple<T>::type;
  template <typename T> using tuple_to_typelist_t = typename tuple_to_typelist<T>::type;
  template <typename T, typename U> struct typelist_embeds: public std::false_type {};
  template <typename... T, typename U>
  struct typelist_embeds<typelist<T...>, U>
	: public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
	sizeof...(T)> {};
  template <typename T, typename E> struct typelist_embeds_any_ref_of: public std::false_type {};
  template <typename U, typename... T>
  struct typelist_embeds_any_ref_of<typelist<T...>, U> : public typelist_embeds<typelist<std::decay_t<T>...>, std::decay_t<U>> {};
}
#endif // TUPLE_HPP
