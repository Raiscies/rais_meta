
#include <rais_meta/list.hpp>
#include <rais_meta/lambda.hpp>
#include <iostream>

using namespace rais::meta;

template <typename... Args>
using l = lambda<op::get, expr<type_list, Args...>, param<0>>;

debug_type<l<int, long, double, void, char>::eval<meta<3>>>;

int main() {
}

