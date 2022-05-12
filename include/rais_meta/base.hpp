#ifndef RAIS_META_BASE_H
#define RAIS_META_BASE_H

#include <cstddef>
#include <type_traits>


namespace rais {
namespace meta {

using std::size_t;

//meta types

struct meta_null {};

template <typename T, T val>
struct meta_object {
	using type = T;
	static constexpr T value = val;
};

template <bool value = false>
using meta_bool = meta_object<bool, value>;


template <typename... Ts>
struct types_pack {
	static constexpr size_t size = 0;
	template <template <typename...> class Templ>
	using cast = Templ<>;
};
template <typename T, typename... Ts>
struct types_pack<T, Ts...> {
	static constexpr size_t size = sizeof...(Ts) + 1;
	template <template <typename...> class Templ>
	using cast = Templ<T, Ts...>;
	using first = T;
};

template <template <typename...> class F>
struct function_warpper {

private:
	template <typename ArgsPack>
	struct unroll_apply_impl {};
	template <template <typename...> class Pack, typename... Args>
	struct unroll_apply_impl<Pack<Args...>> {
		using result = F<Args...>;
	};

public:
	template <typename... Args>
	using apply = F<Args...>;

	template <typename ArgsPack>
	using unroll_apply = typename unroll_apply_impl<ArgsPack>::result;

};
template <template <typename...> class F, typename ArgsPack>
using unroll_apply = typename function_warpper<F>::template unroll_apply<ArgsPack>;


//tools
template <typename T>
using debug_type = typename T::_;

//condition statements
namespace meta_if_detail {

template <bool condition, typename IfTrue, typename... PossibleIfFalse>
struct meta_if_impl {};

//single if - else statement
template <typename IfTrue, typename IfFalse>
struct meta_if_impl<true, IfTrue, IfFalse> {
	using result = IfTrue;
};
template <typename IfTrue, typename IfFalse>
struct meta_if_impl<false, IfTrue, IfFalse> {
	using result = IfFalse;
};
// if - elif - ... elif statement
template <typename IfTrue>
struct meta_if_impl<true, IfTrue> {
	struct result {
		template <bool, typename, typename... NextPossibleIfFalse>
		using elif = typename meta_if_impl<true, IfTrue, NextPossibleIfFalse...>::result;
	};
};
template <typename IfTrue>
struct meta_if_impl<false, IfTrue> {
	struct result {
		template <bool next_condition, typename NextIfTrue, typename... NextPossibleIfFalse>
		using elif = typename meta_if_impl<next_condition, NextIfTrue, NextPossibleIfFalse...>::result;
	};
};	
} // namespace meta_if_detail
template <bool condition, typename IfTrue, typename... PossibleIfFalse>
using meta_if = typename meta_if_detail::meta_if_impl<condition, IfTrue, PossibleIfFalse...>::result;


namespace meta_switch_detail {
template <typename T>
struct meta_switch_impl {
private:

	template <typename K, typename V>
	struct match_impl {

		template <typename NextK, typename NextV>
		using match = match_impl<NextK, NextV>;
		
		template <typename OtherV>
		using mismatch = OtherV;
		
	};

	//the branch that has finished match
	template <typename V>
	struct match_impl<T, V> {

		template <typename NextK, typename NextV>
		using match = match_impl<T, V>;
		
		template <typename OtherV>
		using mismatch = V;
		
	};

public:

	template <typename K, typename V>
	using match = match_impl<K, V>;
	
};

} // namespace meta_switch_detail

template <typename T>
using meta_switch = typename meta_switch_detail::meta_switch_impl<T>;

//loop statement
namespace meta_while_detail {


template <template <typename...> class, template <typename...> class, typename>
struct meta_while_impl_expand_pack;

template <bool condition, template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl {
	//if condition == true
	using result = typename meta_while_impl_expand_pack<Predicate, Function, Function<Context...>>::result;
};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl<false, Predicate, Function, ContextPackTempl, Context...> {
	//if condition == false
	using result = ContextPackTempl<Context...>;
};

template <template <typename...> class Predicate, template <typename...> class Function, typename ContextPack>
struct meta_while_impl_expand_pack {};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl_expand_pack<Predicate, Function, ContextPackTempl<Context...>> {
	using result = typename meta_while_impl<Predicate<Context...>::value, Predicate, Function, ContextPackTempl, Context...>::result;	
};

} // namespace meta_while_detail
template <template <typename...> class Predicate, template <typename...> class Function, typename InitContextPack>
using meta_while = typename meta_while_detail::meta_while_impl_expand_pack<Predicate, Function, InitContextPack>::result;

// template <template <typename...> class Predicate>

} // namespace meta
} // namespace rais


#endif // RAIS_META_BASE_H