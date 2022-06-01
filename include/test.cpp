
#include <utility>
#include <rais_meta/list.hpp>
// #include <rais_meta/map.hpp>

using namespace rais::meta;

// debug_type<op::less<meta<10l>, meta<11>, meta<32>, meta<41>>>;
// debug_type<op::greater_equal<meta<99>, meta<42>, meta<0>, meta<-99>, meta<-99>>>
// debug_type<type_list<int, long, double, char>::push<void>::unshift<void*>::shift::concat<type_list<void***, void*&>>::set<2, const volatile double>::pop>;

// using null_list = type_list<>;

// using list1 = type_list<void*, void**, void***, void****>;

// using has_voidp = list1::contains<void*>;

// using list2 = list1::replace<void****, void**** const>;

// using list3 = list2::replace_if<std::is_const, int>;

// using list4 = type_list<int, double, void, const int, long int, void, char>;


// // debug_type<vlist1>;

// using list5 = list4::erase<void>;

// using list6 = type_list<void, int, char, void, void*, double, long, void, short, decltype(nullptr), void/*, meta_null*/>;

// using splited_list6 = list6::split<void>;

// using splited_vlist1 = vlist1::split<' '>;

// using sliced_list6 = list6::slice<size_t(-3), size_t(-1)>;

// using sliced_vlist1 = vlist1::slice<3>;

// using reversed_list6 = list6::reverse<>;

// using reversed_vlist1 = vlist1::reverse<>;

// debug_type<reversed_vlist1>;

// using vlist1 = value_list<char, ' ', 'a', 'b', ' ', 'c', 'd', '&', ' ', 'G', '3', '*', '#', ' ', ' ', '@',' '>;

// template <typename T>
// struct pred : std::is_same<void, T> {};

// using find_list6 = list6::find<meta_null>;
// using find_vlist1 = vlist1::find<'3'>;
// debug_type<find_vlist1>;


// template <typename IndexSeq>
// struct make_meta_str_helper {};

// template <size_t... index_seq>
// struct make_meta_str_helper<std::integer_sequence<size_t, index_seq...>> {
	
// 	template <typename T>
// 	static constexpr auto make(T t) noexcept{
// 		return value_list<char, T::get()[index_seq]...>{};
// 	}
// };

// template <typename T>
// constexpr auto make_meta_str(T t) noexcept{
// 	return make_meta_str_helper<std::make_index_sequence<sizeof(T::get())>>::make(t);
// }

// #define R_META_STRING(str) (make_meta_str([](){struct temp{static constexpr decltype(auto) get(){return str;}}; return temp{};}()))

// template <typename T, T... chars>
// constexpr auto operator""_metas () noexcept{
// 	return value_list<T, chars...>{};
// }



// #define meta_string(str) decltype(str##_metas)

int main() {
	// auto meta_str = R_META_STRING("Hello meta string!");
	// debug_type<decltype(meta_str)>;
	// debug_type<decltype("Hello meta string!"_metas)>;
	// debug_type<meta_string("hello")>;
	// auto meta_str = make_meta_str("Hello meta string!");
	// auto&& tup = list3::to_tuple<>;
	// auto&& tup = list
	// list4 l;
	return 0;
}



