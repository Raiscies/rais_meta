
#include <utility>
#include <rais_meta/list.hpp>
// #include <rais_meta/map.hpp>

using namespace rais::meta;


template <auto... vs>
struct auto_node {};
template <auto v, auto... vs>
struct auto_node<v, vs...> {
	using value_t = decltype(v);
	using next = auto_node<vs...>;

	static constexpr auto value = v;
};

template <auto... values>
struct auto_list {
	using types = type_list<decltype(values)...>;
	using head = auto_node<values...>;
};


// #define meta_string(str) decltype(str##_metas)
template <typename A, typename B>
using f = types_pack<typename A::mul<B>, B>;
using ans = for_n<3, f, types_pack<meta<7>, meta<3>>>;
debug_type<ans>;
int main() {

	// debug_type<auto_list<1, 'c', -32l, 0x12ul, nullptr>::types>;
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




