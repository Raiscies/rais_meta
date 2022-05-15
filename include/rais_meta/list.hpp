#ifndef RAIS_META_LIST_HPP
#define RAIS_META_LIST_HPP

#include <rais_meta/base.hpp>
#include <tuple>

namespace rais {
namespace meta {

template <typename...>
struct type_node {

	using has_next = meta_bool<false>;
};

template <typename This, typename... Nexts>
struct type_node<This, Nexts...> {
	using type = This;
	using next = type_node<Nexts...>;

	using has_next = meta_bool<sizeof...(Nexts) != 0>;
};

namespace for_range_detail {

template <typename ContextPack, typename Iterator,typename IteratorEnd, typename FunctionWarpper>
using for_range_predicate = typename std::negation<std::is_same<Iterator, IteratorEnd>>;

template <typename ContextPack, typename Iterator, typename IteratorEnd, typename FunctionWarpper>
using for_range_function = types_pack<typename FunctionWarpper::unroll_apply<ContextPack>, typename Iterator::next, IteratorEnd, FunctionWarpper>;

template <typename IteratorBegin, typename IteratorEnd, template <typename...> class Function, typename InitContextPack>
using for_range_impl = typename meta_while<for_range_predicate, for_range_function, types_pack<InitContextPack, IteratorBegin, IteratorEnd, function_warpper<Function>>>::first;

} // namespace for_range_detail

template <typename IteratorBegin, typename IteratorEnd, template <typename...> class Function, typename InitContextPack>
using for_range = typename for_range_detail::for_range_impl<IteratorBegin, IteratorEnd, Function, InitContextPack>;

template <typename Container, template <typename...> class Function, typename InitContextPack>
using for_container = for_range<typename Container::begin, typename Container::end, Function, InitContextPack>;

template <typename... Types>
struct type_list {
	//sizeof...(Types) == 0

	using head = type_node<>;
	static constexpr size_t length = 0;

private:

	template <typename...>
	friend class type_list;

	using self = type_list;

	template <typename... NewLists>
	struct concat_impl {
		using result = self;
	};
	template <typename... NewTypes, typename... NewLists>
	struct concat_impl<type_list<NewTypes...>, NewLists...> {
		using result = typename type_list<NewTypes...>::template concat_impl<NewLists...>::result;
	};

public:

	using begin   = type_node<>;

	using end     = type_node<>;

	template <typename NewType, typename... NewTypes> using push     = type_list<NewType, NewTypes...>;
 
	template <typename NewType, typename... NewTypes> using unshift  = push<NewType, NewTypes...>;

	template <size_t index, typename NewType, typename... NewTypes>  using insert = push<NewType, NewTypes...>;

	template <typename NewList, typename... NewLists> using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <typename Target>                        using contains = meta_bool<false>;

	template <template <typename...> class Function>  using for_each = self;

	template <template <typename...> class Container> using cast     = Container<>;

	template <typename OldType, typename NewType>     using replace  = self;

	template <template <typename> class Predicate, typename NewType> using replace_if = self;

	template <typename ElementType> 
	static constexpr ElementType* to_array = nullptr;

	static constexpr auto to_tuple() noexcept{ return std::tuple<>{}; }
};


template <typename Type, typename... Types>
struct type_list<Type, Types...> {


	using head = type_node<Type, Types...>;
	static constexpr size_t length = sizeof...(Types) + 1;

private:

	template <typename...>
	friend class type_list;

	using self = type_list;

	template <typename CurrentList, typename... Ts>
	struct pop_impl {
		using result = type_list<>;
	};
	template <typename CurrentList, typename T, typename... Ts>
	struct pop_impl<CurrentList, T, Ts...> {
		using result = meta_if<sizeof...(Ts) == 0, 
			CurrentList, 
			typename pop_impl<typename CurrentList::template push<T>, Ts...>::result
		>;
	};

	template <typename...>
	struct concat_impl {
		using result = self;
	};
	template <typename... NewTypes, typename... NewLists>
	struct concat_impl<type_list<NewTypes...>, NewLists...> {
		using result = typename type_list<Type, Types..., NewTypes...>::template concat_impl<NewLists...>::result;
	};


	template <size_t index, size_t current_index = 0, typename CurrentNode = head>
	struct get_impl {
		static_assert(index < length, "target index out of bound");
		using result = typename get_impl<index, current_index + 1, typename CurrentNode::next>::result;
	};
	template <size_t index, typename TargetNode>
	struct get_impl<index, index, TargetNode> {
		using result = typename TargetNode::type;
	};

	template <size_t current_index, size_t index, typename NewType, typename CurrentNode, typename ResultList>
	struct set_impl {
		static_assert(index < length, "target index out of bound");
		using result = typename set_impl<current_index + 1, index, NewType, typename CurrentNode::next, typename ResultList::template push<typename CurrentNode::type> >::result;
	};
	template <size_t index, typename NewType, typename CurrentNode, typename ResultList>
	struct set_impl<index, index, NewType, CurrentNode, ResultList> {
		using result = typename ResultList::template push<NewType>::template concat< unroll_apply<type_list, typename CurrentNode::next> >;
	};

	template <size_t current_index, size_t index, bool insert_on_tail, typename CurrentNode, typename ResultList, typename... NewTypes>
	struct insert_impl {
		//insert_on_tail == false
		using result = typename insert_impl<current_index + 1, index, false, typename CurrentNode::next, typename ResultList::template push<typename CurrentNode::type>, NewTypes... >::result;
	};
	template <size_t index, typename CurrentNode, typename ResultList, typename... NewTypes>
	struct insert_impl<index, index, false, CurrentNode, ResultList, NewTypes...> {
		using result = typename ResultList::template push<NewTypes...>::template concat< unroll_apply<type_list, CurrentNode> >;
	};
	//index >= length
	template <size_t current_index, size_t index, typename CurrentNode, typename ResultList, typename... NewTypes>
	struct insert_impl<current_index, index, true, CurrentNode, ResultList, NewTypes...> {
		using result = type_list<Type, Types..., NewTypes...>;
	};


public:

	template <typename NewType, typename... NewTypes> using push     = type_list<Type, Types..., NewType, NewTypes...>;
 
	template <typename NewType, typename... NewTypes> using unshift  = type_list<NewType, NewTypes..., Type, Types...>;

	template <size_t index, typename NewType, typename... NewTypes>  using insert = typename insert_impl<0, index, (index >= length), head, type_list<>, NewType, NewTypes...>::result;

	template <typename NewList, typename... NewLists> using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <size_t index>                           using get      = typename get_impl<index>::result;

	template <size_t index, typename NewType>         using set      = typename set_impl<0, index, NewType, head, type_list<>>::result;

	template <typename Target>                        using contains = meta_bool<std::is_same<Target, Type>::value || (std::is_same<Target, Types>::value || ...)>;

	template <template <typename...> class Function>  using for_each = type_list<Function<Type>, Function<Types>...>;

	template <template <typename...> class Container> using cast     = Container<Type, Types...>;

	template <typename OldType, typename NewType>     using replace  = type_list< meta_if<std::is_same<OldType, Type>::value, NewType, Type>, meta_if<std::is_same<OldType, Types>::value, NewType, Types>... >;

	template <template <typename> class Predicate, typename NewType> using replace_if = type_list< meta_if< Predicate<Type>::value, NewType, Type>, meta_if< Predicate<Types>::value, NewType, Types>... >;

	using shift    = type_list<Types...>;

	using pop      = typename pop_impl<type_list<>, Type, Types...>::result;

	using begin    = head;

	using end      = type_node<>;

	using front    = Type;

	using back     = get<length - 1>;


	template <typename ElementType> 
	static constexpr ElementType to_array[length] = {static_cast<ElementType>(Type{}), static_cast<ElementType>(Types{})...};

	static constexpr auto to_tuple() { 
		if constexpr(std::is_default_constructible<Type>::value && (std::is_default_constructible<Types>::value && ...))
			return std::tuple<Type, Types...>{Type{}, Types{}...}; 
		else 
			return std::tuple<>{};
	}
};

template <typename ValueT, ValueT... values>
struct value_node {
	using value_t = ValueT;
};

template <typename ValueT, ValueT this_value, ValueT... values>
struct value_node<ValueT, this_value, values...> {
	using value_t = ValueT;
	using next = value_node<value_t, values...>;

	static constexpr value_t value = this_value; 
};

template <typename ValueT, ValueT... values>
struct value_list {
	//sizeof...(values) == 0;
	using value_t = ValueT;
	using head = value_node<value_t>;

	static constexpr size_t length = 0;

	static constexpr value_t* data = nullptr;
private:
	template <typename FriendValueT, FriendValueT...>
	friend class value_list;

	using self = value_list;

	template <typename...>
	struct concat_impl {
		using result = self;
	};
	template <value_t... new_values, typename... NewLists>
	struct concat_impl<value_list<value_t, new_values...>, NewLists...> {
		using result = typename value_list<value_t, new_values...>::template concat_impl<NewLists...>::result;
	};

public:

	template <value_t new_value, value_t... new_values> using push    = value_list<value_t, new_value, new_values...>;
 
	template <value_t new_value, value_t... new_values> using unshift = push<new_value, new_values...>;

	template <size_t index, value_t new_value, value_t... new_values>  using insert = push<new_value, new_values...>;

	template <typename NewList, typename... NewLists> using concat    = typename concat_impl<NewList, NewLists...>::result;

	template <value_t target>                         using contains  = meta_bool<false>;

	template <template <value_t...> class Function>   using for_each  = self;

	template <template <value_t...> class Container>  using cast      = Container<>;

	template <value_t old_value, value_t new_value>   using replace   = self;

	template <template <value_t> class Predicate, value_t new_value> using replace_if = self;

	using begin    = value_node<value_t>;

	using end      = value_node<value_t>;
 
	static constexpr value_t* to_array = nullptr;

	static constexpr auto to_tuple() noexcept{ return std::tuple<>{}; }


};

template <typename ValueT, ValueT value, ValueT... values>
struct value_list<ValueT, value, values...> {
	using value_t = ValueT;
	using head = value_node<value_t, value, values...>;

	static constexpr size_t length = sizeof...(values) + 1;

	static constexpr value_t data[length] = {value, values...};
private:
	template <typename FriendValueT, FriendValueT...>
	friend class value_list;

	using self = value_list;

	template <typename Node>
	struct to_list {};
	template <value_t... node_values> 
	struct to_list<value_node<value_t, node_values...>> {
		using result = value_list<value_t, node_values...>;
	};

	template <size_t current_index, size_t index, bool insert_on_tail, typename CurrentNode, typename ResultList, value_t... new_values>
	struct insert_impl {
		//insert_on_tail == false
		using result = typename insert_impl<current_index + 1, index, false, typename CurrentNode::next, typename ResultList::template push<CurrentNode::value>, new_values... >::result;
	};
	template <size_t index, typename CurrentNode, typename ResultList, value_t... new_values>
	struct insert_impl<index, index, false, CurrentNode, ResultList, new_values...> {
		using result = typename ResultList::template push<new_values...>::template concat< to_list<CurrentNode> >;
	};
	//index >= length
	template <size_t current_index, size_t index, typename CurrentNode, typename ResultList, value_t... new_values>
	struct insert_impl<current_index, index, true, CurrentNode, ResultList, new_values...> {
		using result = value_list<value_t, value, values..., new_values...>;
	};

	template <typename...>
	struct concat_impl {
		using result = self;
	};
	template <value_t... new_values, typename... NewLists>
	struct concat_impl<value_list<value_t, new_values...>, NewLists...> {
		using result = typename value_list<value_t, value, values..., new_values...>::template concat_impl<NewLists...>::result;
	};

	template <size_t current_index, size_t index, value_t new_value, typename CurrentNode, typename ResultList>
	struct set_impl {
		static_assert(index < length, "target index out of bound");
		using result = typename set_impl<current_index + 1, index, new_value, typename CurrentNode::next, typename ResultList::template push<typename CurrentNode::value> >::result;
	};
	template <size_t index, value_t new_value, typename CurrentNode, typename ResultList>
	struct set_impl<index, index, new_value, CurrentNode, ResultList> {
		using result = typename ResultList::template push<new_value>::template concat< to_list<typename CurrentNode::next> >;
	};

	template <typename CurrentList, value_t...>
	struct pop_impl {
		using result = value_list<value_t>;
	};
	template <typename CurrentList, value_t current_value, value_t... current_values>
	struct pop_impl<CurrentList, current_value, current_values...> {
		using result = meta_if<sizeof...(current_values) == 0, 
			CurrentList, 
			typename pop_impl<typename CurrentList::template push<current_value>, current_values...>::result
		>;
	};

public:

	template <value_t new_value, value_t... new_values> using push    = value_list<value_t, value, values..., new_value, new_values...>;
 
	template <value_t new_value, value_t... new_values> using unshift = value_list<value_t, new_value, new_values..., value, values...>;

	template <size_t index, value_t new_value, value_t... new_values>  using insert = typename insert_impl<0, index, (index >= length), head, value_list<value_t>, new_value, new_values...>::result;

	template <typename NewList, typename... NewLists> using concat    = typename concat_impl<NewList, NewLists...>::result;

	static constexpr value_t get(size_t index) noexcept{ return index >= length ? data[length - 1] : data[index]; }
	
	template <size_t index, value_t new_value>        using set       = typename set_impl<0, index, new_value, head, value_list<value_t>>::result;

	template <value_t target>                         using contains  = meta_bool<(target == value) || ((target == values) || ...)>;

	//requires Function<val>::result
	template <template <value_t...> class Function>   using for_each  = value_list<value_t, Function<value>::value, Function<values>::value...>;

	template <template <value_t...> class Container>  using cast      = Container<value, values...>;

	template <value_t old_value, value_t new_value>   using replace   = value_list<value_t, (old_value == value ? new_value : value), (old_value == values ? new_value : values)... >;

	template <template <value_t> class Predicate, value_t new_value> using replace_if = value_list<value_t, (Predicate<value>::value ? new_value : value), (Predicate<values>::value ? new_value : values)... >;

	using shift    = value_list<value_t, values...>;

	using pop      = typename pop_impl<value_list<value_t>, value, values...>::result;

	using begin    = head;

	using end      = value_node<value_t>;

	static constexpr value_t front = value;

	static constexpr value_t back = data[length - 1];
 
	static constexpr value_t* to_array = data;

	static constexpr auto to_tuple() noexcept{ return std::tuple{value, values...}; }


};



} // namespace meta
} // namespace rais

#endif //RAIS_META_LIST_HPP