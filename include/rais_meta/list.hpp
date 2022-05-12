#ifndef RAIS_META_LIST
#define RAIS_META_LIST

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


template <typename... Types>
struct type_list {

	using head = type_node<Types...>;
	static constexpr size_t length = sizeof...(Types);

private:
	template <typename...>
	friend class type_list;

	using self = type_list;

	template <typename...>
	struct shift_impl { using result = type_list<>; };
	template <typename First, typename... Tails>
	struct shift_impl<First, Tails...> { using result = type_list<Tails...>; };

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
		using result = typename type_list<Types..., NewTypes...>::template concat_impl<NewLists...>::result;
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

	//recursion ending
	template <size_t index, typename NewType, typename CurrentNode, typename ResultList>
	struct set_impl<index, index, NewType, CurrentNode, ResultList> {
		using result = typename ResultList::template push<NewType>::template concat< unroll_apply<type_list, typename CurrentNode::next> >;
	};


public:
	                                                  using shift    = typename shift_impl<Types...>::result;

	                                                  using pop      = typename pop_impl<type_list<>, Types...>::result;

	template <typename NewType, typename... NewTypes> using push     = type_list<Types..., NewType, NewTypes...>;
 
	template <typename NewType, typename... NewTypes> using unshift  = type_list<NewType, NewTypes..., Types...>;

	template <typename NewList, typename... NewLists> using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <size_t index>                           using get      = typename get_impl<index>::result;

	template <size_t index, typename NewType>         using set      = typename set_impl<0, index, NewType, head, type_list<>>::result;

	template <typename Target>                        using contains = meta_bool<(std::is_same<Target, Types>::value || ...)>;

	template <template <typename...> class Function>  using for_each = type_list<Function<Types>...>;

	template <template <typename...> class Container> using cast     = Container<Types...>;  

	template <typename OldType, typename NewType>     using replace  = type_list< meta_if<std::is_same<OldType, Types>::value, NewType, Types>... >;

	template <template <typename> class Predicate, typename NewType> using replace_if = type_list< meta_if< Predicate<Types>::value, NewType, Types>... >;


	template <typename ElementType> 
	static constexpr ElementType to_array[sizeof...(Types)] = {static_cast<ElementType>(Types{})...};

	static constexpr auto to_tuple() { 
		if constexpr((std::is_default_constructible<Types>::value && ...))
			return std::tuple<Types...>{Types{}...}; 
		else 
			return std::tuple<>{};
	} 

};

} // namespace meta
} // namespace rais

#endif //RAIS_META_LIST