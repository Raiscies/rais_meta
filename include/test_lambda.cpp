
// #include <rais_meta/list.hpp>
#include <rais_meta/lambda.hpp>

using namespace rais::meta;

// using v1 = op::add<std::integral_constant<long, 9>, meta<100>, meta<10>>;

// template <typename A, typename B>
using less_f = fn<op::less>;

using binded_div = bind<type_list, param<4>, param<1>, param<0>>;

// debug_type<binded_div::eval<void, int, long, double, short>>;

// debug_type<less_f::apply<meta<3>, meta<54>, meta<6>>::closure<>>;

using l = lambda<op::if_, expr<std::is_same, param<0>, param<1>>, param<2>, param<3>>;

int main() {

	using result = typename l::eval<void, decltype(nullptr), meta_bool<true>, meta_bool<false>>;

	debug_type<result>;
	return 0;
}


