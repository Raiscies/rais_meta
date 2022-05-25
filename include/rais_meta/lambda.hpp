#ifndef RAIS_META_LAMBDA_HPP
#define RAIS_META_LAMBDA_HPP

#include <rais_meta/base.hpp>
#include <rais_meta/list.hpp>

namespace rais {
namespace meta {



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