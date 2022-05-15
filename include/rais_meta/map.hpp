#ifndef RAIS_META_MAP_HPP
#define RAIS_META_MAP_HPP

#include "rais_meta/list.hpp"


namespace rais {
namespace meta {


template <typename Key, typename Value>
struct kv_pair {

	using key = Key;
	using value = Value;
};

template <typename KeyT, typename ValueT, KeyT Key, ValueT Value>
using warpper_pair = kv_pair< meta_object<KeyT, Key>, meta_object<ValueT, Value> >;

namespace dynamic_get_detail {

template <typename KeyT, typename ValueT, typename KVPairNode>
struct dynamic_get_impl {

	[[gnu::always_inline]] static inline constexpr ValueT impl(KeyT key) noexcept{
		using this_pair = typename KVPairNode::type;
		return key == this_pair::key::value ? static_cast<ValueT>(this_pair::value::value) :
			dynamic_get_impl<KeyT, ValueT, typename KVPairNode::next>::impl(key);
	}
};

template <typename KeyT, typename ValueTWithoutPtr, typename KVPairNode>
struct dynamic_get_impl<KeyT, ValueTWithoutPtr*, KVPairNode> {
	[[gnu::always_inline]] static inline constexpr ValueTWithoutPtr* impl(KeyT key) noexcept{
		using this_pair = typename KVPairNode::type;
		return key == this_pair::key::value ? static_cast<ValueTWithoutPtr*>(this_pair::value::value) :
			dynamic_get_impl<KeyT, ValueTWithoutPtr*, typename KVPairNode::next>::impl(key);
	}
};


template <typename KeyT, typename ValueT>
struct dynamic_get_impl<KeyT, ValueT, type_node<> > {
	[[gnu::always_inline]] static inline constexpr ValueT impl(KeyT key) noexcept{
		//key not found
		return static_cast<ValueT>(-1);
	}
};

template <typename KeyT, typename ValueTWithoutPtr>
struct dynamic_get_impl<KeyT, ValueTWithoutPtr*, type_node<> > {
	[[gnu::always_inline]] static inline constexpr ValueTWithoutPtr* impl(KeyT key) noexcept{
		//key not found
		return nullptr;
	}
};

} //namespace dynamic_get_detail

template <typename KeyT, typename ValueT, typename KVPairNode>
[[gnu::always_inline]] inline constexpr ValueT dynamic_get(KeyT key) noexcept{
	return dynamic_get_detail::dynamic_get_impl<KeyT, ValueT, KVPairNode>::impl(key);
} 

namespace map_check_detail {


template <typename T, typename KeyT, typename ValueT>
struct is_warpper_pair_impl {
	using result = meta_bool<false>;
};

template <typename KeyT, typename ValueT, KeyT Key, ValueT Value>
struct is_warpper_pair_impl <kv_pair<meta_object<KeyT, Key>, meta_object<ValueT, Value> >, KeyT, ValueT > {
	using result = meta_bool<true>;
};

template <typename KeyT, typename ValueT, KeyT Key, ValueT Value>
struct is_warpper_pair_impl <kv_pair<meta_object<KeyT, Key>, meta_object<ValueT, Value> >, void, ValueT > {
	using result = meta_bool<true>;
};

template <typename KeyT, typename ValueT, KeyT Key, ValueT Value>
struct is_warpper_pair_impl <kv_pair<meta_object<KeyT, Key>, meta_object<ValueT, Value> >, KeyT, void > {
	using result = meta_bool<true>;
};

template <typename KeyT, typename ValueT, KeyT Key, ValueT Value>
struct is_warpper_pair_impl <kv_pair<meta_object<KeyT, Key>, meta_object<ValueT, Value> >, void, void > {
	using result = meta_bool<true>;
};

template <typename T, typename KeyT, typename ValueT>
using is_warpper_pair = typename is_warpper_pair_impl<T, KeyT, ValueT>::result;

template <typename KeyT, typename ValueT, typename... KVPairs>
struct warpper_pair_check_impl {
	//KVPairs must be kv_pair<meta_object<KeyT, Key>, meta_object<ValueT, Value>>
	using result = meta_bool<(is_warpper_pair<KVPairs, KeyT, ValueT>::value && ...)>;

};

template <typename T, typename Key, typename Value>
struct is_type_pair_impl {
	using result = meta_bool<false>;
};

template <typename KeyT, KeyT Key, typename Value>
struct is_type_pair_impl<kv_pair<meta_object<KeyT, Key>, Value>, KeyT, void> {
	using result = meta_bool<true>;
};

template <typename Key, typename ValueT, ValueT Value>
struct is_type_pair_impl<kv_pair<Key, meta_object<ValueT, Value>>, void, ValueT> {
	using result = meta_bool<true>;
};

} //namespace pair_check_detail

template <typename ThisPair, typename ThisKeyList, typename HasRepeatedKey>
using key_check_callback = meta_if< HasRepeatedKey::value || ThisKeyList::template contains<typename ThisPair::key>::value, 
	type_node< typename ThisKeyList::shift, meta_bool<true> >, 
	type_node< typename ThisKeyList::shift, HasRepeatedKey > 		
>;

template <typename KeyT, typename ValueT, typename... KVPairs>
using warpper_pair_check = typename map_check_detail::warpper_pair_check_impl<KeyT, ValueT, KVPairs...>::result;

template <typename T, typename KeyT, typename ValueT>
using is_type_pair = typename map_check_detail::is_type_pair_impl<T, KeyT, ValueT>::result;


template <typename... KVPairs>
class type_map {

public:
	using container = type_list<KVPairs...>;

private:
	static_assert( (is_instantiated_from<KVPairs, kv_pair>::value && ...), 
		"invalid type_map: unexcepted type as kv_pair" );

	static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
		"invalid type_map: exists repeating keys" );

	using self = type_map<KVPairs...>;

	template <typename ThisPair, typename Key, typename Result>
	using get_pair_callback = meta_if< std::is_same<typename ThisPair::key, Key>::value && std::is_same<Result, void>::value, /* 当前键匹配且之前的键没有匹配的 */
		type_node<Key, ThisPair>,
		type_node<Key, Result>
	>;


	template <typename Key>
	struct get_impl {
		using result = typename for_container<container, get_pair_callback, type_node<Key, void> >::next::type;

		static_assert(!std::is_same<result, void>::value, "key not found");
	};

	template <typename ThisPair, typename Key, typename... RemainingPairs>
	using erase_callback = meta_if< std::is_same<typename ThisPair::key, Key>::value, 
			type_list<Key, RemainingPairs...>, 
			type_list<Key, RemainingPairs..., ThisPair>
	>;

	template <typename ThisMap, typename ResultMap>
	struct merge_callback_impl {
		using result = type_list<ResultMap>;
	};

	template <typename... PairsOfThisMap, typename... PairsOfResultMap>
	struct merge_callback_impl<type_map<PairsOfThisMap...>, type_map<PairsOfResultMap...>> {
		using result = type_list<type_map<PairsOfResultMap..., PairsOfThisMap...>>;
	};

	template <typename ThisMap, typename ResultMap>
	using merge_callback = typename merge_callback_impl<ThisMap, ResultMap>::result;

	template <typename ThisPair, typename Key, typename Result>
	using contains_callback = meta_if<Result::value || std::is_same<typename ThisPair::key, Key>::value, type_node<Key, meta_bool<true>>, type_node<Key, meta_bool<false>>>;

	template <typename Key, typename Value, bool has_key>
	struct set_impl {
		//has_key == false
		using result = type_map<KVPairs..., kv_pair<Key, Value> >;
	};

	template <typename Key, typename Value>
	struct set_impl<Key, Value, true> {
		//has_key == true
	private:
		template <typename Pair>
		using is_target = std::is_same<typename Pair::key, Key>;
	public:
		using result = typename container::template replace_if< is_target, kv_pair<Key, Value> >::template cast<type_map>;
	};


public:

	static constexpr size_t size = sizeof...(KVPairs);	

	template <typename Key>                 using get      = typename get_impl<Key>::result::value;

	template <typename Key>                 using contains = typename for_container< container, contains_callback, type_node<Key, meta_bool<false>> >::next::type;

	//if Key is not exist then put the kv pair, otherwise alter the kv pair
	template <typename Key, typename NewValue> using set   = typename set_impl<Key, NewValue, contains<Key>::value >::result;

	template <typename Key, typename Value> using put      = type_map<KVPairs..., kv_pair<Key, Value>>;

	template <typename Key>                 using erase    = typename for_container< container, erase_callback, type_list<Key> >::shift::template cast<type_map>;

	template <typename... Maps>             using merge    = typename for_container< type_list<Maps...>, merge_callback, type_list<self> >::first_element;

	template <template <typename...> class Container> using cast       = Container<KVPairs...>;
	template <template <typename...> class Container> using cast_key   = Container<typename KVPairs::key...>;
	template <template <typename...> class Container> using cast_value = Container<typename KVPairs::value...>;

	template <template <typename...> class F>         using for_each   = type_map<F<KVPairs>...>; 

};


template <typename KeyT = void, typename ValueT = void>
struct bind_type {
	//KeyT != void && ValueT != void

	using key_t   = KeyT;
	using value_t = ValueT;

	template <key_t Key>     using key_warpper   = meta_object<key_t, Key>;
	template <value_t Value> using value_warpper = meta_object<value_t, Value>;
	template <key_t Key, value_t Value> using binded_warpper_pair = warpper_pair<key_t, value_t, Key, Value>;
	
	template <typename... KVPairs>
	class value_map {


		//限制KVPairs的类型
		static_assert( warpper_pair_check<key_t, value_t, KVPairs...>::value, 
			"invalid value_map: exists bad kv_pair" );

		static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
			"invalid value_map: exists repeating keys" );

		//KVPair is kv_pair<meta_object<key_t, key>, meta_object<value_t, value>>
	
	public:
		using inner_map = ::rais::meta::type_map<KVPairs...>;
		using key_t = KeyT;
		using value_t = ValueT;


		static constexpr size_t size = sizeof...(KVPairs);

		template <key_t Key> static constexpr value_t get() noexcept{ return inner_map::template get<key_warpper<Key>>::value; }

		static constexpr value_t get(key_t key) noexcept{ return dynamic_get<key_t, value_t, typename inner_map::container::head >(key); } 

		template <key_t Key>                using get_warpper = typename inner_map::template get<key_warpper<Key>>;

		template <key_t Key, value_t Value> using set         = typename inner_map::template set<key_warpper<Key>, value_warpper<Value> >::template cast< bind_type::value_map >;

		template <key_t Key, value_t Value> using put         = typename bind_type::template value_map<KVPairs..., binded_warpper_pair<Key, Value> >;

		template <key_t Key>                using erase       = typename inner_map::template erase<key_warpper<Key>>::template cast< bind_type::value_map >;

		template <typename... Maps>         using merge       = typename inner_map::template merge<Maps...>::template cast< bind_type::value_map >;

		template <key_t Key>                using contains    = typename inner_map::template contains<key_warpper<Key>>;

		template <template <typename...> class Container> using cast               = Container<KVPairs...>;
		template <template <typename...> class Container> using cast_key_warpper   = Container<typename KVPairs::key...>;
		template <template <typename...> class Container> using cast_value_warpper = Container<typename KVPairs::value...>;
		template <template <key_t...>    class Container> using cast_key           = Container<KVPairs::key::value...>;
		template <template <value_t...>  class Container> using cast_value         = Container<KVPairs::value::value...>;

		template <template <typename...> class F>         using for_each           = bind_type::value_map<F<KVPairs>...>;

	};

};

template <typename KeyT>
struct bind_type<KeyT, void> {
	//KeyT != void && ValueT == void

	using key_t   = KeyT;
	using value_t = void;


	template <key_t Key> using key_warpper = meta_object<key_t, Key>;
	template <key_t Key, typename ValueT, ValueT Value> using binded_warpper_pair = warpper_pair<key_t, ValueT, Key, Value>;
	

	template <typename... KVPairs>
	class value_map {

		static_assert( warpper_pair_check<key_t, value_t, KVPairs...>::value, 
			"invalid value_map: exists bad kv_pair" );

		static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
			"invalid value_map: exists repeating keys" );

		//KVPair is kv_pair<meta_object<key_t, key>, meta_object<value_t, value>>
	
	public:
		using inner_map = ::rais::meta::type_map<KVPairs...>;
		using key_t = KeyT;
		using value_t = void;

		static constexpr size_t size = sizeof...(KVPairs);

		template <key_t Key> static constexpr auto get() noexcept -> typename inner_map::template get<key_warpper<Key>>::type { return inner_map::template get<key_warpper<Key>>::value; } 

		template <typename ValueT> static constexpr get(key_t key) noexcept{ return dynamic_get<KeyT, ValueT, typename inner_map::container::head>(key); } 

		template <key_t Key>                                using get_warpper = typename inner_map::template get<key_warpper<Key>>;

		template <key_t Key, typename ValueT, ValueT Value> using set         = typename inner_map::template set<key_warpper<Key>, meta_object<ValueT, Value> >::template cast< bind_type::value_map >;

		template <key_t Key, typename ValueT, ValueT Value> using put         = typename bind_type::template value_map<KVPairs..., binded_warpper_pair<Key, ValueT, Value> >;

		template <key_t Key>                                using erase       = typename inner_map::template erase<key_warpper<Key>>::template cast< bind_type::value_map >;

		template <typename... Maps>                         using merge       = typename inner_map::template merge<Maps...>::template cast< bind_type::value_map >;

		template <key_t Key>                                using contains    = typename inner_map::template contains<key_warpper<Key>>;

		template <template <typename...> class Container> using cast                   = Container<KVPairs...>;
		template <template <typename...> class Container> using cast_key_wapper        = Container<typename KVPairs::key...>;
		template <template <typename...> class Container> using cast_value_warpper     = Container<typename KVPairs::value...>;
		template <template <key_t...>    class Container> using cast_key               = Container<KVPairs::key::value...>;

		template <template <typename...> class F>         using for_each               = bind_type::value_map<F<KVPairs>...>;

	};

	template <typename... KVPairs>
	class type_map {

		//keys(KeyT) -> types

		static_assert( (is_type_pair<KVPairs, key_t, void>::value && ...), 
			"invalid type_map: exists bad kv_pair" );

		static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
			"invalid type_map: exists repeating keys" );


		//KVPair is kv_pair<meta_object<key_t, key>, Value>
	
	public:
		using inner_map = ::rais::meta::type_map<KVPairs...>;
		using key_t = KeyT;
		using value_t = void;

		static constexpr size_t size = sizeof...(KVPairs);

		template <key_t Key>                                using get      = typename inner_map::template get<key_warpper<Key>>;

		template <key_t Key, typename Value>                using set      = typename inner_map::template set< key_warpper<Key>, Value >::template cast< bind_type::type_map >;

		template <key_t Key, typename Value>                using put      = typename bind_type::template type_map<KVPairs..., kv_pair<meta_object<KeyT, Key>, Value > >;

		template <key_t Key>                                using erase    = typename inner_map::template erase<key_warpper<Key>>::template cast< bind_type::type_map >;

		template <typename... Maps>                         using merge    = typename inner_map::template merge<Maps...>::template cast< bind_type::type_map >;

		template <key_t Key>                                using contains = typename inner_map::template contains<key_warpper<Key>>;

		template <template <typename...> class Container> using cast                   = Container<KVPairs...>;
		template <template <typename...> class Container> using cast_key_wapper        = Container<typename KVPairs::key...>;
		template <template <typename...> class Container> using cast_value             = Container<typename KVPairs::value...>;
		template <template <key_t...>    class Container> using cast_key               = Container<KVPairs::key::value...>;

		template <template <typename...> class F>         using for_each               = bind_type::type_map<F<KVPairs>...>;
	};
};

template <typename ValueT>
struct bind_type<void, ValueT> {
	//KeyT == void && ValueT != void

	using key_t   = void;
	using value_t = ValueT;

	template <value_t Value> using value_warpper = meta_object<value_t, Value>;
	template <typename KeyT, KeyT Key, value_t Value> using binded_warpper_pair = warpper_pair<KeyT, value_t, Key, Value>;
	
	template <typename... KVPairs>
	class value_map {

		static_assert( warpper_pair_check<key_t, value_t, KVPairs...>::value, 
			"invalid value_map: exists bad kv_pair" );

		static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
			"invalid value_map: exists repeating keys" );

		//KVPair is kv_pair<meta_object<key_t, key>, meta_object<value_t, value>>
	
	public:
		using inner_map = ::rais::meta::type_map<KVPairs...>;
		using key_t = void;
		using value_t = ValueT;

		static constexpr size_t size = sizeof...(KVPairs);

		template <typename KeyT, KeyT Key> static constexpr value_t get() noexcept{ return inner_map::template get<meta_object<KeyT, Key>>::value; }

		template <typename KeyT> static constexpr value_t get(KeyT key) noexcept{ return dynamic_get<KeyT, value_t, typename inner_map::container::head>(key); }

		template <typename KeyT, KeyT Key>                using get_warpper = typename inner_map::template get<meta_object<KeyT, Key>>;

		template <typename KeyT, KeyT Key, value_t Value> using set         = typename inner_map::template set<meta_object<KeyT, Key>, value_warpper<Value> >::template cast< bind_type::value_map >;

		template <typename KeyT, KeyT Key, value_t Value> using put         = typename bind_type::template value_map<KVPairs..., binded_warpper_pair<KeyT, Key, Value> >;

		template <typename KeyT, KeyT Key>                using erase       = typename inner_map::template erase<meta_object<KeyT, Key>>::template cast< bind_type::value_map >;

		template <typename... Maps>                       using merge       = typename inner_map::template merge<Maps...>::template cast< bind_type::value_map >;

		template <typename KeyT, KeyT Key>                using contains    = typename inner_map::template contains<meta_object<KeyT, Key>>;

		template <template <typename...> class Container> using cast               = Container<KVPairs...>;
		template <template <typename...> class Container> using cast_key_warpper   = Container<typename KVPairs::key...>;
		template <template <typename...> class Container> using cast_value_warpper = Container<typename KVPairs::value...>;
		template <template <value_t...>  class Container> using cast_value         = Container<KVPairs::value::value...>;
	
		template <template <typename...> class F>         using for_each           = bind_type::value_map<F<KVPairs>...>;

	};

	template <typename... KVPairs>
	class type_map {

		static_assert( (is_type_pair<KVPairs, void, value_t>::value && ...), 
			"invalid type_map: exists bad kv_pair" );

		static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
			"invalid value_map: exists repeating keys" );

		//KVPair is kv_pair<Key, meta_object<ValueT, Value> >
	
	public:
		using inner_map = ::rais::meta::type_map<KVPairs...>;
		using key_t = void;
		using value_t = ValueT;

		static constexpr size_t size = sizeof...(KVPairs);

		template <typename Key>                using get      = typename inner_map::template get<Key>;

		template <typename Key, value_t Value> using set      = typename inner_map::template set<Key, value_warpper<Value> >::template cast< bind_type::type_map >;

		template <typename Key, value_t Value> using put      = typename bind_type::template type_map<KVPairs..., kv_pair<Key, meta_object<value_t, Value> > >;

		template <typename Key>                using erase    = typename inner_map::template erase<Key>::template cast< bind_type::type_map >;

		template <typename... Maps>            using merge    = typename inner_map::template merge<Maps...>::template cast< bind_type::type_map >;

		template <typename Key>                using contains = typename inner_map::template contains<Key>;

		template <template <typename...> class Container> using cast               = Container<KVPairs...>;
		template <template <typename...> class Container> using cast_key           = Container<typename KVPairs::key...>;
		template <template <typename...> class Container> using cast_value_warpper = Container<typename KVPairs::value...>;
		template <template <value_t...>  class Container> using cast_value         = Container<KVPairs::value::value...>;

		template <template <typename...> class F>         using for_each           = bind_type::type_map<F<KVPairs>...>;

	};
};

template <>
struct bind_type<void, void> {
//KeyT == void && ValueT == void

	using key_t   = void;
	using value_t = void;

	template <typename... KVPairs>
	class value_map {

		static_assert( warpper_pair_check<key_t, value_t, KVPairs...>::value, 
			"invalid value_map: exists bad kv_pair" );

		static_assert( !(for_container<type_list<KVPairs...>, key_check_callback, type_node< typename type_list<typename KVPairs::key...>::shift, meta_bool<false> > >::next::type::value), 
			"invalid value_map: exists repeating keys" );

		//KVPair is kv_pair<meta_object<key_t, key>, meta_object<value_t, value>>
	
	public:
		using inner_map = ::rais::meta::type_map<KVPairs...>;
		using key_t = void;
		using value_t = void;

		static constexpr size_t size = sizeof...(KVPairs);

		template <typename KeyT, KeyT Key> static constexpr auto get() -> typename inner_map::template get<meta_object<KeyT, Key>>::type { return inner_map::template get<meta_object<KeyT, Key>>::value; }

		template <typename KeyT, typename ValueT> static constexpr get(KeyT key) noexcept{ return dynamic_get<KeyT, ValueT, typename inner_map::container::head>(key); } 

		template <typename KeyT, KeyT Key>                using get_warpper         = typename inner_map::template get<meta_object<KeyT, Key>>;

		template <typename KeyT, KeyT Key, typename ValueT, ValueT Value> using set = typename inner_map::template set< meta_object<KeyT, Key>, meta_object<ValueT, Value> >::template cast< bind_type::value_map >;

		template <typename KeyT, KeyT Key, typename ValueT, ValueT Value> using put = typename bind_type::template value_map<KVPairs..., warpper_pair<KeyT, ValueT, Key, Value> >;

		template <typename KeyT, KeyT Key>                using erase               = typename inner_map::template erase<meta_object<KeyT, Key>>::template cast< bind_type::value_map >;

		template <typename... Maps>                       using merge               = typename inner_map::template merge<Maps...>::template cast< bind_type::value_map >;

		template <typename KeyT, KeyT Key>                using contains            = typename inner_map::template contains<meta_object<KeyT, Key>>;

		template <template <typename...> class Container> using cast                = Container<KVPairs...>;
		template <template <typename...> class Container> using cast_key_warpper    = Container<typename KVPairs::key...>;
		template <template <typename...> class Container> using cast_value_warpper  = Container<typename KVPairs::value...>;

		template <template <typename...> class F>         using for_each            = typename bind_type::value_map<F<KVPairs>...>;

	};
};

namespace to_value_map_detail {
	

enum class cast_flag {
	KEY_ONLY,		//key_t == void && value_t != void
	VALUE_ONLY,		//key_t != void && value_t == void
	KEY_AND_VALUE	//key_t == void && value_t == void
};

template <cast_flag Flag, typename SourceIterator, typename ResultKVPairList>
struct cast_helper {
//unspecified: cast_flag == KEY_AND_VALUE

	//is a kv_pair's iterator
	//src::type is kv_pair
	using src = SourceIterator;
	//is a kv_pair's list
	using res = ResultKVPairList;

	//cast flag
	static constexpr auto flag = Flag;
	// static constexpr auto flag = is_meta_object<typename src::type::key>::value ? 
	// 	(is_meta_object<typename src::type::value>::value ? cast_flag::KEY_AND_VALUE : cast_flag::KEY_ONLY)
	// 	: cast_flag::VALUE_ONLY; 
		
private:
	template <typename src::type::key Key, typename src::type::value Value>
	struct put_impl {
		using result = cast_helper<flag, typename src::next, 
			typename res::template push<
				kv_pair<
					meta_object<typename src::type::key, Key>, 
					meta_object<typename src::type::value, Value>
				>
			>
		>;
	};

public:
	template <typename src::type::key Key, typename src::type::value Value>
	using put = typename put_impl<Key, Value>::result;

}; //class cast_helper

//KEY_ONLY branch
template <typename SourceIterator, typename ResultKVPairList>
struct cast_helper<cast_flag::KEY_ONLY, SourceIterator, ResultKVPairList> {

	using src = SourceIterator;
	using res = ResultKVPairList;
	static constexpr auto flag = cast_flag::KEY_ONLY;		
	
private:
	template <typename src::type::key Key>
	struct put_impl {
		using result = cast_helper<flag, typename src::next, 
			typename res::template push<
				kv_pair<
					meta_object<typename src::type::key, Key>, 
					typename src::type::value
				>
			>
		>;
	};

public:
	template <typename src::type::key Key>
	using put = typename put_impl<Key>::result;

}; //class cast_helper

//VALUE_ONLY branch
template <typename SourceIterator, typename ResultKVPairList>
struct cast_helper<cast_flag::VALUE_ONLY, SourceIterator, ResultKVPairList> {

	using src = SourceIterator;
	using res = ResultKVPairList;
	static constexpr auto flag = cast_flag::VALUE_ONLY;

private:	
	template <typename src::type::value Value>
	struct put_impl {
		using result = cast_helper<flag, typename src::next, 
			typename res::template push<
				kv_pair<
					typename src::type::key, 
					 meta_object<typename src::type::value, Value> 
				>
			>
		>;
	};

public:
	template <typename src::type::value Value>
	using put = typename put_impl<Value>::result;

}; //class cast_helper


//KEY_AND_VALUE: ending branch
template <template <typename...> class SourceIteratorTempl, typename TailPair, typename ResultKVPairList>
struct cast_helper<cast_flag::KEY_AND_VALUE, SourceIteratorTempl<TailPair>, ResultKVPairList> {
	
	using src = SourceIteratorTempl<TailPair>;
	using res = ResultKVPairList;

	static constexpr auto flag = cast_flag::KEY_AND_VALUE;
	
private:
	//the last element
	template <typename src::type::key Key, typename src::type::value Value>
	struct put_impl {
		using result = typename res::template push<
			kv_pair<
				meta_object<typename src::type::key, Key>, 
				meta_object<typename src::type::value, Value>
			>
		>::template cast<bind_type<typename src::type::key::type, typename src::type::value::type>::template value_map >;
	};

public:
	template <typename src::type::key Key, typename src::type::value Value>
	using put = typename put_impl<Key, Value>::result;

}; //class cast_helper


//KEY_ONLY ending branch
template <template <typename...> class SourceIteratorTempl, typename TailPair, typename ResultKVPairList>
struct cast_helper<cast_flag::KEY_ONLY, SourceIteratorTempl<TailPair>, ResultKVPairList> {
	
	using src = SourceIteratorTempl<TailPair>;
	using res = ResultKVPairList;

	static constexpr auto flag = cast_flag::KEY_ONLY;
	
private:
	//the final element
	template <typename src::type::key Key>
	struct put_impl {
		using result = typename res::template push<
			kv_pair<
				meta_object<typename src::type::key, Key>,
				typename src::type::value 
			>
		>::template cast<bind_type<void, typename src::type::value::type>::template value_map >;
	};

public:
	template <typename src::type::key Key>
	using put = typename put_impl<Key>::result;

}; //class cast_helper


//VALUE_ONLY ending branch
template <template <typename...> class SourceIteratorTempl, typename TailPair, typename ResultKVPairList>
struct cast_helper<cast_flag::VALUE_ONLY, SourceIteratorTempl<TailPair>, ResultKVPairList> {
	
	using src = SourceIteratorTempl<TailPair>;
	using res = ResultKVPairList;

	static constexpr auto flag = cast_flag::VALUE_ONLY;
	
private:

	//the last element
	template <typename src::type::value Value >
	struct put_impl {
		using result = typename res::template push<
			kv_pair<
				typename src::type::key, 
				meta_object<typename src::type::value, Value>
			>
		>::template cast< bind_type<typename src::type::key::type , void>::template value_map >;
	};

public:
	template <typename src::type::value Value>
	using put = typename put_impl<Value>::result;

}; //class cast_helper

template<typename TypeMap>
struct to_value_map_impl {
	using inner_map = TypeMap;
	using key_t   = typename inner_map::key_t;
	using value_t = typename inner_map::value_t;

	constexpr static auto flag = std::is_same<key_t, void>::value ? 
		(std::is_same<value_t, void>::value ? cast_flag::KEY_AND_VALUE : cast_flag::KEY_ONLY )
		: cast_flag::VALUE_ONLY;

	using result = cast_helper<flag, typename inner_map::template cast<type_node>, type_list<> >;

};

template <template <typename...> class TypeMapTempl >
struct to_value_map_impl<TypeMapTempl<> > {
	using inner_map = TypeMapTempl<>;
	using key_t   = typename inner_map::key_t;
	using value_t = typename inner_map::value_t;

	using result = typename bind_type<key_t, value_t>::template value_map<>;
};


} //namespace to_value_map_detail 

template <typename TypeMap>
using to_value_map = typename to_value_map_detail::to_value_map_impl<TypeMap>::result;


} //namespace meta
	
} //namespace rais


#endif //RAIS_META_MAP_HPP