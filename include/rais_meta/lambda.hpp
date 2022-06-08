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
	struct closure_impl {
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
	using closure = typename closure_impl<Args...>::result;

	template <typename... Args>
	using apply = typename apply_impl<(sizeof...(Args) + current_argument_count >= full_argument_count), Args...>::result;

};


template <template <typename...> class F, typename ArgsList = type_list<>, typename Options = function_options<>>
using fn = function<F, ArgsList, Options>;

namespace bind_detail {

//to detect arg placeholder
struct placeholder_flag {};

//as argument placeholders
template <size_t N>
struct arg: placeholder_flag {
	static constexpr size_t n = N;
}; 

template <typename T>
using args_filter = meta_if<std::is_base_of_v<placeholder_flag, T>, T, arg<0>>;

template <typename ArgsList, typename ReplaceArgsList, typename N>
using arg_map = types_pack<typename ArgsList::replace<arg<N::value>, typename ReplaceArgsList::front>, typename ReplaceArgsList::shift<>, typename N::inc>;

template <template <typename...> class F, typename... BindedArgs>
struct binded_function {
	static constexpr size_t max_placeholder = max(args_filter<BindedArgs>::n...);
	using args_list = type_list<BindedArgs...>;

private:
	template <typename... Args>
	struct closure_impl {
	private:
		using temp = for_n<max_placeholder + 1, arg_map, types_pack<args_list, type_list<Args...>, meta<0>>>;
	public:
		using result = unroll_apply<F, typename temp::first::template concat<typename temp::shift::first> >; 
	};

public:
	template <typename... Args>
	using closure = typename closure_impl<Args...>::result;
};

} //namespace bind_detail

template <size_t N>
using arg = typename bind_detail::arg<N>;

template <template <typename...> class F, typename... BindedArgs>
using bind = typename bind_detail::binded_function<F, BindedArgs...>;


} //namespace meta
} //namespace rais


#endif //RAIS_META_LAMBDA_HPP

