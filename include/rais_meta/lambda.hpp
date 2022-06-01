#ifndef RAIS_META_LAMBDA_HPP
#define RAIS_META_LAMBDA_HPP

#include <rais_meta/base.hpp>
#include <rais_meta/list.hpp>

namespace rais {
namespace meta {

namespace meta_function_detail {

//as argument placeholders
template <size_t N>
struct arg {
	static constexpr size_t n = N;
}; 

template <size_t arg_count = -1/* unlimited argument count */>
struct function_options {
	static constexpr size_t argument_count = arg_count;
};


template <template <typename...> class F, typename ArgsList = type_list<>, typename Options = function_options<>>
struct meta_function {
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
		using result = meta_function<F, typename args_list::push<Args...>, options>;
	};

public:

	template <typename... Args>
	using push_args  = meta_function<F, args_list::push<Args...>, options>;
	template <typename... Args>
	using reset_args = meta_function<F, type_list<Args...>, options>;

	template <typename... Args>
	using result = typename warpper::unroll_apply<typename args_list::push<Args...>>;

	template <typename... Args>
	using apply = typename apply_impl<(sizeof...(Args) + current_argument_count >= full_argument_count), Args...>::result;

};

} //namespace meta_function_detail

} //namespace meta
} //namespace rais


#endif //RAIS_META_LAMBDA_HPP

