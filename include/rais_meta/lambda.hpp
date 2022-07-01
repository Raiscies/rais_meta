#ifndef RAIS_META_LAMBDA_HPP
#define RAIS_META_LAMBDA_HPP

#include <rais_meta/base.hpp>
#include <rais_meta/list.hpp>

namespace rais {

template <typename T1, typename T2, typename... Ts>
constexpr auto max(const T1& a, const T2& b, const Ts&... args) noexcept{
	if constexpr(sizeof...(Ts) == 0) {
		return a < b ? b : a;
	}else {
		return max(a < b ? b : a, args...);
	}
}
template <typename T1, typename T2, typename... Ts>
constexpr auto min(const T1& a, const T2& b, const Ts&... args) noexcept{
	if constexpr(sizeof...(Ts) == 0) {
		return b < a ? b : a;
	}else {
		return min(b < a ? b : a, args...);
	}
}

namespace meta {

//option class
template <size_t arg_count = -1/* unlimited argument count */>
struct function_options {
	static constexpr size_t argument_count = arg_count;
};

template <template <typename...> class F, typename ArgsList = type_list<>, typename Options = function_options<>>
struct function {
	using args_list = ArgsList;
	using warpper   = function_warpper<F>;
	using options   = Options;

	static constexpr size_t current_argument_count = args_list::length;
	static constexpr size_t full_argument_count = options::argument_count;

private:
	template <bool has_enough_args, typename... Args>
	struct apply_impl {
		//has_enough_args == true
		using result = typename warpper::unroll_apply<typename args_list::push<Args...>>;
	};
	template <typename... Args>
	struct apply_impl<false, Args...> {
		//has_enough_args == false
		using result = function<F, typename args_list::push<Args...>, options>;
	};

	template <typename... Args>
	struct push_args_impl {
		using result = function<F, typename args_list::push<Args...>, options>;
	};
	template <typename... Args>
	struct eval_impl {
		using result = typename warpper::unroll_apply<typename args_list::push<Args...>>;
	};

public:

	template <typename... Args>
	using push_args  = typename push_args_impl<Args...>::result;
	template <typename... Args>
	using reset_args = function<F, type_list<Args...>, options>;
	template <typename NewOptions>
	using reset_options = function<F, args_list, NewOptions>;

	template <typename... Args>
	using eval = typename eval_impl<Args...>::result;

	template <typename... Args>
	using apply = typename apply_impl<(sizeof...(Args) + current_argument_count >= full_argument_count), Args...>::result;

};

template <template <typename...> class F, typename ArgsList = type_list<>, typename Options = function_options<>>
using fn = function<F, ArgsList, Options>;

namespace bind_detail {

//to detect parameter placeholder
struct placeholder_flag {};

//as argument placeholders
template <size_t N>
struct param: placeholder_flag {
	static constexpr size_t n = N;
}; 

template <typename T>
using arg_filter = meta_if<std::is_base_of_v<placeholder_flag, T>, T, param<0>>;

template <typename ArgsList, typename ReplaceArgsList, typename N>
using arg_map = types_pack<typename ArgsList::replace<param<N::value>, typename ReplaceArgsList::front>, typename ReplaceArgsList::shift<>, typename N::inc>;

template <template <typename...> class F, typename... BindedArgs>
struct binded_function {
	static constexpr size_t max_placeholder = max(arg_filter<BindedArgs>::n...);
	using args_list = type_list<BindedArgs...>;

private:
	template <typename... Args>
	struct eval_impl {
	private:
		static_assert(sizeof...(Args) > max_placeholder, "too few arguments");
		using temp = for_n<max_placeholder + 1, arg_map, types_pack<args_list, type_list<Args...>, meta<0>>>;
	public:
		using result = unroll_apply<F, typename temp::first::template concat<typename temp::shift::first> >; 
	};

public:
	template <typename... Args>
	using eval = typename eval_impl<Args...>::result;
};

} //namespace bind_detail

//placeholder type
template <size_t N>
using param = typename bind_detail::param<N>;

template <typename T>
using is_placeholder = meta_bool<std::is_base_of_v<bind_detail::placeholder_flag, T>>;

template <template <typename...> class F, typename... BindedArgs>
using bind = typename bind_detail::binded_function<F, BindedArgs...>;


namespace lambda_detail {


enum class argument_category {
	placeholder, 
	expression,
	binded_type
};

struct expression_flag {};

template <template <typename...> class F, typename... SubExpressions>
struct expression_node: expression_flag {
	using fn = function<F>;
	using sub_expression_list = type_list<SubExpressions...>;

private:
	
	template <typename Arg, typename ReplaceArgsList, argument_category category>
	struct replace_placeholder_impl {};
	template <typename Arg, typename ReplaceArgsList>
	struct replace_placeholder_impl<Arg, ReplaceArgsList, argument_category::placeholder> {
		//Arg is placeholder
		using result = typename ReplaceArgsList::get<Arg::n>;
	};
	template <typename Arg, typename ReplaceArgsList>
	struct replace_placeholder_impl<Arg, ReplaceArgsList, argument_category::expression> {
		//Arg is expression
		using result = typename Arg::template eval<ReplaceArgsList>;
	};
	template <typename Arg, typename ReplaceArgsList>
	struct replace_placeholder_impl<Arg, ReplaceArgsList, argument_category::binded_type> {
		//Arg is binded type
		using result = Arg;
	};

	template <typename Arg, typename ReplaceArgsList>
	using replace_placeholder = typename replace_placeholder_impl<Arg, ReplaceArgsList, 
		is_placeholder<Arg>::value              ? argument_category::placeholder :
		std::is_base_of_v<expression_flag, Arg> ? argument_category::expression  : 
		                                          argument_category::binded_type
	>::result;

public:
	template <typename ReplaceArgsList>
	using eval = F<replace_placeholder<SubExpressions, ReplaceArgsList>...>;
};


template <template <typename...> class F, typename... SubExpressions>
struct lambda {
	using root_node = expression_node<F, SubExpressions...>;
	using outer_fn = function<F>;
	using sub_expression_list = type_list<SubExpressions...>;

	template <typename... ReplaceArgs>
	using eval = typename root_node::eval<type_list<ReplaceArgs...>>;
};

} //namespace lambda_detail

template <template <typename...> class F, typename... SubExpressions>
using lambda = lambda_detail::lambda<F, SubExpressions...>;

template <template <typename...> class F, typename... SubExpressions>
using expr = lambda_detail::expression_node<F, SubExpressions...>;


} //namespace meta
} //namespace rais


#endif //RAIS_META_LAMBDA_HPP
