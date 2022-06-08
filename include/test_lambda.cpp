
// #include <rais_meta/list.hpp>
#include <rais_meta/lambda.hpp>

using namespace rais::meta;

// using v1 = op::add<std::integral_constant<long, 9>, meta<100>, meta<10>>;

// template <typename A, typename B>
using less_f = fn<op::less>;

using binded_div = bind<op::div, arg<1>, arg<0>>;

debug_type<binded_div::closure<meta<9>, meta<78>>>;

// debug_type<less_f::apply<meta<3>, meta<54>, meta<6>>::closure<>>;

int main() {
	return 0;
}