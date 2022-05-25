#ifndef RAIS_META_LAMBDA_HPP
#define RAIS_META_LAMBDA_HPP

#include <rais_meta/base.hpp>
#include <rais_meta/list.hpp>

namespace rais {
namespace meta {


//operators
namespace op {

namespace details {

template <typename Value>
struct exchange_value_impl{};

template <typename T, T v, template <typename, T> class Templ>
struct exchange_value_impl<Templ<T, v>> {
	template <T new_val>
	using result = Templ<T, new_val>;

};

} //namespace details

template <typename Value, auto new_val>
using exchange_val = typename details::exchange_value_impl<Value>::result<new_val>;

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


} //namespace op


template <typename C, typename T, typename F>
using if_then_else = meta_if<C::value, T, F>;
namespace fn_detail {

template <template <typename...> class F, typename... Args>
struct fn {
	using args_list = type_list<Args...>;



};

} //namespace fn_detail

} //namespace meta
} //namespace rais

#endif //RAIS_META_LAMBDA_HPP