
#include <rais_meta/list.hpp>
#include <rais_meta/lambda.hpp>

using namespace rais::meta;

using v1 = op::add<std::integral_constant<long, 9>, meta<100>, meta<10>>;

debug_type<v1>;

int main() {
	return 0;
}