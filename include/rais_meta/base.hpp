#ifndef RAIS_META_BASE_HPP
#define RAIS_META_BASE_HPP

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

template <auto value> 
using meta = meta_object<decltype(value), value>;

template <typename T, T val> 
struct meta_integer: meta_object<T, val> {
	using typename meta_object<T, val>::type;
	using meta_object<T, val>::value;
	
	template <typename Val, typename... Vals>
	using add = meta_integer<type, ((value + Val::value) + ... + Vals::value)>;
	template <typename Val, typename... Vals>
	using sub = meta_integer<type, ((value - Val::value) - ... - Vals::value)>;
	template <typename Val, typename... Vals>
	using mul = meta_integer<type, ((value * Val::value) * ... * Vals::value)>;
	template <typename Val, typename... Vals>
	using div = meta_integer<type, ((value / Val::value) / ... / Vals::value)>;
	template <typename Val, typename... Vals>
	using mod = meta_integer<type, ((value % Val::value) % ... % Vals::value)>;

	using inc = meta_integer<T, value + 1>;
	using dec = meta_integer<T, value - 1>;
	
	template <typename Val, typename... Vals>
	using bit_and = meta_integer<type, ((value & Val::value) & ... & Vals::value)>;
	template <typename Val, typename... Vals>
	using bit_or  = meta_integer<type, ((value | Val::value) | ... | Vals::value)>;
	template <typename Val, typename... Vals>
	using bit_xor = meta_integer<type, ((value ^ Val::value) ^ ... ^ Vals::value)>;

	using bit_compl = meta_integer<T, ~value>;
	
};

template <bool val = false>
struct meta_bool: meta_object<bool, val> {
	using typename meta_object<bool, val>::type;
	using meta_object<bool, val>::value;

	template <typename Val, typename... Vals>
	using logic_and = meta_bool<((value && Val::value) && ... && Vals::value)>;

	template <typename Val, typename... Vals>
	using logic_or = meta_bool<((value || Val::value) || ... || Vals::value)>;

	using logic_not = meta_bool<!value>;

};

template <char Value = '\0'>
using meta_char = meta_object<char, Value>;
template <signed char Value = 0>
using meta_schar = meta_object<signed char, Value>;
template <unsigned char Value = 0>
using meta_uchar = meta_object<unsigned char, Value>;

template <short Value = 0>
using meta_short = meta_object<short, Value>;
template <unsigned short Value = 0>
using meta_ushort = meta_object<unsigned short, Value>;

template <int Value = 0>
using meta_int = meta_object<int, Value>;
template <unsigned int Value = 0>
using meta_uint = meta_object<unsigned int, Value>;

template <long Value = 0L>
using meta_long = meta_object<long, Value>;
template <unsigned long Value = 0UL>
using meta_ulong = meta_object<unsigned long, Value>;

template <long long Value = 0LL>
using meta_llong = meta_object<long long, Value>;
template <unsigned long long Value = 0ULL>
using meta_ullong = meta_object<unsigned long long, Value>;

template <size_t val>
using meta_size_t = meta_object<size_t, val>;


template <typename... Ts>
struct types_pack {
	static constexpr size_t size = 0;

	template <template <typename...> class Templ>
	using cast = Templ<>;

	template <typename... NewTypes>   using unshift = types_pack<NewTypes...>;
	template <typename... NewTypes>   using push    = types_pack<NewTypes...>;
	template <size_t shift_count = 1> using shift   = types_pack<>;
};
template <typename T, typename... Ts>
struct types_pack<T, Ts...> {
	static constexpr size_t size = sizeof...(Ts) + 1;
	template <template <typename...> class Templ>
	using cast = Templ<T, Ts...>;
	using first = T;

	//some simple function
	template <typename... NewTypes>   using unshift = types_pack<NewTypes..., T, Ts...>;
	template <typename... NewTypes>   using push    = types_pack<T, Ts..., NewTypes...>;
	using shift   = types_pack<Ts...>;
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
template <typename ValueT>
struct nontype_param {
	using value_t = ValueT;

	template <template <value_t, typename...> class F> //with one non-type param
	struct function_warpper {
	using value_t = ValueT;

	private:
		template <typename ArgsPack>
		struct unroll_apply_impl {};
		template <template <typename...> class TypesPack, typename ValueWarpper, typename... Args>
		struct unroll_apply_impl<TypesPack<ValueWarpper, Args...>> {
			using result = F<ValueWarpper::value, Args...>;
		};

	public:
		template <value_t value, typename... Args>
		using apply = F<value, Args...>;

		template <typename ArgsPack>
		using unroll_apply = typename unroll_apply_impl<ArgsPack>::result;

	};

	template <template <value_t, typename...> class F, typename ArgsPack>
	using unroll_apply = typename function_warpper<F>::template unroll_apply<ArgsPack>;

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
		
		template <typename FallbackV>
		using mismatch = FallbackV;
		
	};

	//the branch that has finished match
	template <typename V>
	struct match_impl<T, V> {

		template <typename NextK, typename NextV>
		using match = match_impl<T, V>;
		
		template <typename FallbackV>
		using mismatch = V;
		
	};

public:

	template <typename K, typename V>
	using match = match_impl<K, V>;
	
};

} // namespace meta_switch_detail

template <typename T>
using meta_switch = typename meta_switch_detail::meta_switch_impl<T>;

namespace meta_args_detail {

template <typename... Args>
struct meta_args_impl {

private:
	template <bool condition, template <typename...> class IfTrueFunction, template <typename...> class... PossibleIfFalseFunction>
	struct invoke_if_impl {};
	template <template <typename...> class IfTrueFunction, template <typename...> class IfFalseFunction>
	struct invoke_if_impl<true, IfTrueFunction, IfFalseFunction> {
		using result = IfTrueFunction<Args...>;
	};
	template <template <typename...> class IfTrueFunction, template <typename...> class IfFalseFunction>
	struct invoke_if_impl<false, IfTrueFunction, IfFalseFunction> {
		using result = IfFalseFunction<Args...>;
	};
	template <template <typename...> class IfTrueFunction>
	struct invoke_if_impl<true, IfTrueFunction> {
		struct result {
			template <bool next_condition, template <typename...> class NextIfTrueFunction, template <typename...> class... NextPossibleIfFalseFunction>
			using elif = typename invoke_if_impl<true, IfTrueFunction, NextPossibleIfFalseFunction...>::result;
		};
	};
	template <template <typename...> class IfTrueFunction>
	struct invoke_if_impl<false, IfTrueFunction> {
		struct result {
			template <bool next_condition, template <typename...> class NextIfTrueFunction, template <typename...> class... NextPossibleIfFalseFunction>
			using elif = typename invoke_if_impl<next_condition, NextIfTrueFunction, NextPossibleIfFalseFunction...>::result;
		};
	};

	template <typename T>
	struct invoke_switch_impl {
	private:

		template <typename K, template <typename...> class F>
		struct match_impl {

			template <typename NextK, template <typename...> class NextF>
			using match = match_impl<NextK, NextF>;
			
			template <template <typename...> class FallbackF>
			using mismatch = FallbackF<Args...>;
			
		};
		template <template <typename...> class F>
		struct match_impl<T, F> {
			template <typename NextK, template <typename...> class NextF>
			using match = match_impl<T, F>;
			
			template <template <typename...> class FallbackF>
			using mismatch = F<Args...>;
		};
	public:
		template <typename K, template <typename...> class F>
		using match = match_impl<K, F>;
	};

	template <bool condition, template <typename...> class IfTrueFunction, template <typename...> class... PossibleIfFalseFunction>
	using invoke_if = typename invoke_if_impl<condition, IfTrueFunction, PossibleIfFalseFunction...>::result;

	template <typename T>
	using invoke_switch = invoke_switch_impl<T>;

};

} //namespace meta_args_detail

template <typename... Args>
using meta_args = typename meta_args_detail::meta_args_impl<Args...>;


//loop statement
namespace meta_while_detail {

struct base_break_loop {};

template <typename ContextPack>
struct break_loop: base_break_loop { 
	using pack = ContextPack; 
};

template <typename ContextPack>
using is_break_loop_impl = meta_bool<std::is_base_of_v<base_break_loop, ContextPack>>; 

template <template <typename...> class, template <typename...> class, typename>
struct meta_while_impl_expand_pack;

template <bool condition, bool is_break_loop, template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl {};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl<true, false, Predicate, Function, ContextPackTempl, Context...> {
	//if condition == true
	using result = typename meta_while_impl_expand_pack<Predicate, Function, Function<Context...>>::result;
};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl<false, false, Predicate, Function, ContextPackTempl, Context...> {
	//if condition == false
	using result = ContextPackTempl<Context...>;
};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename> class BreakLoop, typename... ContextPack>
struct meta_while_impl<true, true, Predicate, Function, BreakLoop, ContextPack...> {
	//break loop
	using result = typename BreakLoop<ContextPack...>::pack;
};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename> class BreakLoop, typename... ContextPack>
struct meta_while_impl<false, true, Predicate, Function, BreakLoop, ContextPack...> {
	//break loop anyway
	using result = typename BreakLoop<ContextPack...>::pack;
};

template <template <typename...> class Predicate, template <typename...> class Function, typename ContextPack>
struct meta_while_impl_expand_pack {};
template <template <typename...> class Predicate, template <typename...> class Function, template <typename...> class ContextPackTempl, typename... Context>
struct meta_while_impl_expand_pack<Predicate, Function, ContextPackTempl<Context...>> {
	using result = typename meta_while_impl<Predicate<Context...>::value, is_break_loop_impl<ContextPackTempl<Context...>>::value, Predicate, Function, ContextPackTempl, Context...>::result;	
};

} // namespace meta_while_detail
template <template <typename...> class Predicate, template <typename...> class Function, typename InitContextPack>
using meta_while = typename meta_while_detail::meta_while_impl_expand_pack<Predicate, Function, InitContextPack>::result;

template <typename ContextPack>
using break_loop = meta_while_detail::break_loop<ContextPack>;


template <typename Pack>
using is_break_loop = meta_while_detail::is_break_loop_impl<Pack>;

//template traits
namespace is_same_template_detail {

template <template <typename...> class T, template <typename...> class U, template <typename...> class... Templs>
struct is_same_template_impl {
	using result = meta_bool<false>;
};
template <template <typename...> class T, template <typename...> class... Templs>
struct is_same_template_impl<T, T, Templs...> {
	using result = typename is_same_template_impl<T, Templs...>::result;
};
template <template <typename...> class T>
struct is_same_template_impl <T, T> {
	using result = meta_bool<true>;
};

} //namespace is_same_template_detail

namespace is_instantiated_from_detail {

template <typename T, template <typename...> class Templ> 
struct is_instantiated_from_impl {
	using result = meta_bool<false>;	
};
template <template <typename...> class Templ, typename... Args>
struct is_instantiated_from_impl<Templ<Args...>, Templ> {
	using result = meta_bool<true>;
};

} //nemaspace is_instantiated_from_detail

//不支持含有非类型模板参数或模板模板参数的模板
template <template <typename...> class... Templs>
using is_same_template = typename is_same_template_detail::is_same_template_impl<Templs...>::result;

template <typename T, template <typename...> class Templ> 
using is_instantiated_from = typename is_instantiated_from_detail::is_instantiated_from_impl<T, Templ>::result;



} // namespace meta
} // namespace rais


#endif // RAIS_META_BASE_HPP