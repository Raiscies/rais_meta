
#include <rais_meta/list.hpp>
#include <rais_meta/map.hpp>

using namespace rais::meta;

// debug_type<type_list<int, long, double, char>::push<void>::unshift<void*>::shift::concat<type_list<void***, void*&>>::set<2, const volatile double>::pop>;

using null_list = type_list<>;

using list1 = type_list<void*, void**, void***, void****>;

using has_voidp = list1::contains<void*>;

using list2 = list1::replace<void****, void**** const>;

using list3 = list2::replace_if<std::is_const, int>;

using list4 = type_list<int, double, void, const int, long int, void, char>;

using vlist1 = value_list<char, ' ', 'a', 'b', ' ', 'c', 'd', '&', ' ', 'G', '3', '*', '#', ' ', ' ', '@',' '>;

// debug_type<vlist1>;

using list5 = list4::erase<void>;

using list6 = type_list<void, int, char, void, void*, double, long, void, short, decltype(nullptr), void>;

using splited_list6 = list6::split<void>;

using splited_vlist1 = vlist1::split<' '>;

using sliced_list6 = list6::slice<size_t(-3), size_t(-1)>;

using sliced_vlist1 = vlist1::slice<3>;

debug_type<sliced_vlist1>;


int main() {
	// auto&& tup = list3::to_tuple<>;
	// auto&& tup = list
	// list4 l;
	return 0;
}



