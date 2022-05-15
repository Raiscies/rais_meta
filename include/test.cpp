
#include <rais_meta/list.hpp>
#include <rais_meta/map.hpp>

using namespace rais::meta;

// debug_type<type_list<int, long, double, char>::push<void>::unshift<void*>::shift::concat<type_list<void***, void*&>>::set<2, const volatile double>::pop>;

using null_list = type_list<>;

using list1 = type_list<void*, void**, void***, void****>;

using has_voidp = list1::contains<void*>;

using list2 = list1::replace<void****, void**** const>;

using list3 = list2::replace_if<std::is_const, int>;

using list4 = type_list<int, double, void, const int>;

// debug_type<list3>;


int main() {
	// auto&& tup = list3::to_tuple<>;
	// auto&& tup = list
	// list4 l;
	return 0;
}