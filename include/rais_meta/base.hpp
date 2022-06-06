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
	using bit_and    = meta_integer<type, ((value & Val::value) & ... & Vals::value)>;
	template <typename Val, typename... Vals>
	using bit_or     = meta_integer<type, ((value | Val::value) | ... | Vals::value)>;
	template <typename Val, typename... Vals>
	using bit_xor    = meta_integer<type, ((value ^ Val::value) ^ ... ^ Vals::value)>;
	template <typename Val, typename... Vals>
	using bit_lshift = meta_integer<type, ((value << Val::value) << ... << Vals::value)>;
	template <typename Val, typename... Vals>
	using bit_rshift = meta_integer<type, ((value >> Val::value) >> ... >> Vals::value)>;

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

//operators
namespace op {

namespace exchange_value_details {

template <typename Value>
struct exchange_value_impl {};

template <typename T, T v, template <typename, T> class Templ>
struct exchange_value_impl<Templ<T, v>> {
	template <T new_val>
	using result = Templ<T, new_val>;

};

} //namespace exchange_value_details

template <typename Value, auto new_val>
using exchange_val = typename exchange_value_details::exchange_value_impl<Value>::result<new_val>;

template <typename Value, typename... Values>
using add = exchange_val<Value, (Value::value + ... + Values::value)>;
template <typename Value, typename... Values>
using sub = exchange_val<Value, (Value::value - ... - Values::value)>;
template <typename Value, typename... Values>
using mul = exchange_val<Value, (Value::value * ... * Values::value)>;
template <typename Value, typename... Values>
using div = exchange_val<Value, (Value::value / ... / Values::value)>;
template <typename Value, typename... Values>
using mod = exchange_val<Value, (Value::value % ... % Values::value)>;

template <typename Value, typename... Values>
using bit_and = exchange_val<Value, (Value::value & ... & Values::value)>;
template <typename Value, typename... Values>
using bit_or  = exchange_val<Value, (Value::value | ... | Values::value)>;
template <typename Value, typename... Values>
using bit_xor = exchange_val<Value, (Value::value ^ ... ^ Values::value)>;
template <typename Value, typename... Values>
using bit_lshift  = exchange_val<Value, (Value::value << ... << Values::value)>;
template <typename Value, typename... Values>
using bit_rshift  = exchange_val<Value, (Value::value >> ... >> Values::value)>;

template <typename Value, typename... Values>
using logic_and = exchange_val<Value, (Value::value && ... && Values::value)>;
template <typename Value, typename... Values>
using logic_or  = exchange_val<Value, (Value::value || ... || Values::value)>;

template <typename Value>
using inc = exchange_val<Value, Value::value + 1>;
template <typename Value>
using dec = exchange_val<Value, Value::value - 1>;
template <typename Value>
using neg       = exchange_val<Value, -Value::value>;
template <typename Value>
using logic_not = exchange_val<Value, !Value::value>;
template <typename Value>
using bit_compl = exchange_val<Value, ~Value::value>;

//comparators
namespace comparators_detail {

#define R_CREATE_COMPARATOR(name, op)   \
	template <typename ValueT, ValueT... vals> \
	struct name##_impl {                \
		template <ValueT v1, ValueT v2, ValueT... values> \
		struct impl {                   \
			static constexpr bool result = ((v1 op v2) && impl<v2, values...>::result); \ 
		};                              \
		template <ValueT v1, ValueT v2> \
		struct impl<v1, v2> {           \
			static constexpr bool result = (v1 op v2); \
		};                              \
		static constexpr bool result = impl<vals...>::result; \
	};

R_CREATE_COMPARATOR(less,           <)
R_CREATE_COMPARATOR(greater,        >)
R_CREATE_COMPARATOR(less_equal,    <=)
R_CREATE_COMPARATOR(greater_equal, >=)
R_CREATE_COMPARATOR(equal,         ==)
R_CREATE_COMPARATOR(not_equal,     !=) //note: it would compare the adjant values only

#undef R_CREATE_COMPARATOR
} //namespace comparators_detail

template <typename Value, typename... Values>
using less          = meta_bool<comparators_detail::less_impl         <decltype(Value::value), Value::value, Values::value...>::result>;
template <typename Value, typename... Values>
using greater       = meta_bool<comparators_detail::greater_impl      <decltype(Value::value), Value::value, Values::value...>::result>;
template <typename Value, typename... Values>
using less_equal    = meta_bool<comparators_detail::less_equal_impl   <decltype(Value::value), Value::value, Values::value...>::result>;
template <typename Value, typename... Values>
using greater_equal = meta_bool<comparators_detail::greater_equal_impl<decltype(Value::value), Value::value, Values::value...>::result>;
template <typename Value, typename... Values>
using equal         = meta_bool<comparators_detail::equal_impl        <decltype(Value::value), Value::value, Values::value...>::result>;
template <typename Value, typename... Values>
using not_equal     = meta_bool<comparators_detail::not_equal_impl    <decltype(Value::value), Value::value, Values::value...>::result>;


} //namespace op

namespace auto_meta_detail {

template<auto value, bool is_integral, bool is_bool>
struct auto_meta_impl {
	//any type
	using result = meta_object<decltype(value), value>;
};
template<auto value>
struct auto_meta_impl<value, true, false> {
	//is integer type
	using result = meta_integer<decltype(value), value>;
};
template<auto value>
struct auto_meta_impl<value, true, true> {
	//is boolean type
	using result = meta_bool<value>;
};


} //namespace auto_meta_detail
 
//auto meta type
template <auto value> 
using meta = typename auto_meta_detail::auto_meta_impl<value, std::is_integral_v<decltype(value)>, std::is_same_v<bool, std::remove_cv_t<decltype(value)>>>::result;

template <char Value = '\0'>
using meta_char = meta_integer<char, Value>;
template <signed char Value = 0>
using meta_schar = meta_integer<signed char, Value>;
template <unsigned char Value = 0>
using meta_uchar = meta_integer<unsigned char, Value>;

template <short Value = 0>
using meta_short = meta_integer<short, Value>;
template <unsigned short Value = 0>
using meta_ushort = meta_integer<unsigned short, Value>;

template <int Value = 0>
using meta_int = meta_integer<int, Value>;
template <unsigned int Value = 0>
using meta_uint = meta_integer<unsigned int, Value>;

template <long Value = 0L>
using meta_long = meta_integer<long, Value>;
template <unsigned long Value = 0UL>
using meta_ulong = meta_integer<unsigned long, Value>;

template <long long Value = 0LL>
using meta_llong = meta_integer<long long, Value>;
template <unsigned long long Value = 0ULL>
using meta_ullong = meta_integer<unsigned long long, Value>;

template <size_t val>
using meta_size_t = meta_integer<size_t, val>;


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

	template <typename... Args>
	struct apply_impl {
		using result = F<Args...>;
	};

public:
	template <typename... Args>
	using apply = typename apply_impl<Args...>::result;

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

struct break_loop_flag {};

template <typename ContextPack>
struct break_loop: break_loop_flag { 
	using pack = ContextPack; 
};

template <typename ContextPack>
using is_break_loop_impl = meta_bool<std::is_base_of_v<break_loop_flag, ContextPack>>; 

template <bool is_break_loop, typename ContextPack>
struct remove_break_warp_impl{ using result = typename ContextPack::pack; };
template <typename ContextPack>
struct remove_break_warp_impl<false, ContextPack> { using result = ContextPack; };
template <typename ContextPack>
using remove_break_warp = typename remove_break_warp_impl<is_break_loop_impl<ContextPack>::value, ContextPack>::result;


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

template <typename ContextPack, typename I, typename N, typename FunctionWarpper>
using for_n_predicate = meta_bool<(I::value < N::value) and !is_break_loop_impl<ContextPack>::value>;
template <typename ContextPack, typename I, typename N, typename FunctionWarpper>
using for_n_function  = types_pack<typename FunctionWarpper::unroll_apply<ContextPack>, typename I::inc, N, FunctionWarpper>;

} // namespace meta_while_detail
template <typename ContextPack>
using break_loop = meta_while_detail::break_loop<ContextPack>;

template <typename Pack>
using is_break_loop = meta_while_detail::is_break_loop_impl<Pack>;

template <typename ContextPack>
using remove_break_warp = meta_while_detail::remove_break_warp<ContextPack>;

template <template <typename...> class Predicate, template <typename...> class Function, typename InitContextPack>
using meta_while = typename meta_while_detail::meta_while_impl_expand_pack<Predicate, Function, InitContextPack>::result;

template <size_t n, template <typename...> class Function, typename InitContextPack>
using for_n = remove_break_warp<typename meta_while<meta_while_detail::for_n_predicate, meta_while_detail::for_n_function, types_pack<InitContextPack, meta_size_t<0>, meta_size_t<n>, function_warpper<Function>>>::first>;


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