#ifndef RAIS_META_LIST_HPP
#define RAIS_META_LIST_HPP

#include <rais_meta/base.hpp>

namespace rais {
namespace meta {

enum class list_category {
	type_list, 
	value_list
};

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
using for_range_predicate = meta_bool<!is_break_loop<ContextPack>::value and !std::is_same_v<Iterator, IteratorEnd>>;

template <typename ContextPack, typename Iterator, typename IteratorEnd, typename FunctionWarpper>
using for_range_function = types_pack<typename FunctionWarpper::unroll_apply<typename ContextPack::template unshift<typename Iterator::type>>, typename Iterator::next, IteratorEnd, FunctionWarpper>;


template <typename IteratorBegin, typename IteratorEnd, template <typename...> class Function, typename InitContextPack>
using for_range_impl = remove_break_warp<typename meta_while<for_range_predicate, for_range_function, types_pack<InitContextPack, IteratorBegin, IteratorEnd, function_warpper<Function>>>::first>;

//for_value_range_detail

template <typename ContextPack, typename Iterator, typename IteratorEnd, typename FunctionWarpper>
using for_value_range_function = types_pack<typename FunctionWarpper::template unroll_apply<typename ContextPack::template unshift<meta<Iterator::value>>>, typename Iterator::next, IteratorEnd, FunctionWarpper>;

template <typename IteratorBegin, typename IteratorEnd, template <auto, typename...> class Function, typename InitContextPack>
using for_value_range_impl = remove_break_warp<typename meta_while<for_range_predicate, for_value_range_function, types_pack<InitContextPack, IteratorBegin, IteratorEnd, typename nontype<1>::function_warpper<Function>> >::first>;


} // namespace for_range_detail

template <typename IteratorBegin, typename IteratorEnd, template <typename I, typename... Contexts> class Function, typename InitContextPack>
using for_range = typename for_range_detail::for_range_impl<IteratorBegin, IteratorEnd, Function, InitContextPack>;

template <typename Container, template <typename I, typename... Contexts> class Function, typename InitContextPack>
using for_container = for_range<typename Container::begin, typename Container::end, Function, InitContextPack>;

template <typename IteratorBegin, typename IteratorEnd, template <auto I, typename... Contexts> class Function, typename InitContextPack>
using for_value_range = typename for_range_detail::for_value_range_impl<IteratorBegin, IteratorEnd, Function, InitContextPack>;

template <typename Container, template <auto I, typename... Contexts> class Function, typename InitContextPack>
using for_value_container = for_value_range<typename Container::begin, typename Container::end, Function, InitContextPack>;

template <typename... Types>
struct type_list {
	//sizeof...(Types) == 0

	using head = type_node<>;
	static constexpr size_t length = 0;

	static constexpr list_category category = list_category::type_list;

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

	template <typename any_type = meta_null>          using reverse  = self;

	template <size_t shift_count = 1>                 using shift    = self;

	template <size_t pop_count = 1>                   using pop      = self;

	template <typename... NewTypes>                   using push     = type_list<NewTypes...>;
 
	template <typename... NewTypes>                   using unshift  = push<NewTypes...>;

	template <typename Target>                        using erase    = type_list<>;

	template <size_t index>                           using erase_by_index = type_list<>;

	template <template <typename> class Predicate>    using erase_if = type_list<>;

	template <size_t index, typename... NewTypes>     using insert = push<NewTypes...>;

	template <typename NewList, typename... NewLists> using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <typename Target>                        using contains = meta_bool<false>;

	template <template <typename> class Predicate>    using find_if  = type_node<>;

	template <typename Target>                        using find     = type_node<>;

	template <template <typename...> class Function>  using for_each = self;

	template <template <typename...> class Container> using cast     = Container<>;

	template <typename OldType, typename NewType>     using replace  = self;

	template <template <typename> class Predicate, typename NewType> using replace_if = self;

	template <typename Separator>                     using split    = self;

	template <size_t from, size_t to>                 using slice    = self;

	// template <typename ElementType> 
	// static constexpr ElementType* to_array = nullptr;

};


template <typename Type, typename... Types>
struct type_list<Type, Types...> {


	using head = type_node<Type, Types...>;
	static constexpr size_t length = sizeof...(Types) + 1;

	static constexpr list_category category = list_category::type_list;

private:

	template <typename...>
	friend class type_list;

	using self = type_list;

	template <size_t pop_count, typename CurrentList, typename... Ts>
	struct pop_impl {
		using result = CurrentList;
	};
	template <size_t pop_count, typename CurrentList, typename T, typename... Ts>
	struct pop_impl<pop_count, CurrentList, T, Ts...> {
		using result = typename meta_if<pop_count == 0, 
			self
		>::template elif< sizeof...(Ts) == pop_count,  
			typename CurrentList::template push<T>, 
			typename pop_impl<pop_count, typename CurrentList::template push<T>, Ts...>::result
		>;
	};

	//using alias('using' statement) will trig a complier bug: CWG1430
	//see https://stackoverflow.com/questions/30707011/pack-expansion-for-alias-template
	template <typename I, typename CurrentList, typename PredicateWarpper> 
	struct erase_if_f: types_pack<meta_if<PredicateWarpper::template apply<I>::value, CurrentList, typename CurrentList::template push<I> >, PredicateWarpper> {};

	template <typename I, typename CurrentList, typename Index, typename CurrentIndex>
	struct erase_by_index_f: types_pack<meta_if<Index::value == CurrentIndex::value, CurrentList, typename CurrentList::template push<I>>, Index, typename CurrentIndex::inc> {};

	template <typename I, typename CurrentList, typename Target>
	struct erase_f: types_pack<meta_if<std::is_same<I, Target>::value, CurrentList, typename CurrentList::template push<I>>, Target> {};

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

	template <typename I, typename ResultList, typename Separator>
	struct split_f: types_pack< meta_if<std::is_same_v<I, Separator>, 
		typename ResultList::push<type_list<>>, 
		typename ResultList::set<ResultList::length - 1, typename ResultList::back::template push<I> > 
	>, Separator> {};

	template <typename I, typename ResultList>
	struct reverse_f: types_pack<typename ResultList::unshift<I>> {};

	template <typename>
	struct reverse_impl: for_range<head, type_node<>, reverse_f, types_pack<type_list<>>> {};

	template <typename I, typename CurrentNode, typename PredicateWarpper>
	struct find_if_f: meta_if<PredicateWarpper::template apply<I>::value, break_loop<types_pack<CurrentNode, PredicateWarpper>>, types_pack<typename CurrentNode::next, PredicateWarpper> > {};

	template <typename Target>
	struct find_pred {
		template <typename I>
		struct f: std::is_same<Target, I> {};
	};

public:

	using begin    = head;

	using end      = type_node<>;

	template <size_t shift_count = 1>                 using shift    = meta_if<shift_count == 0, self, typename type_list<Types...>::shift<shift_count - 1>>;

	template <size_t pop_count = 1>                   using pop      = typename pop_impl<pop_count, type_list<>, Type, Types...>::result;

	template <typename... NewTypes>                   using push     = type_list<Type, Types..., NewTypes...>;
 
	template <typename... NewTypes>                   using unshift  = type_list<NewTypes..., Type, Types...>;

	template <size_t index, typename... NewTypes>     using insert   = typename insert_impl<0, index, (index >= length), head, type_list<>, NewTypes...>::result;

	template <typename Target>                        using erase    = typename for_range<begin, end, erase_f, types_pack<type_list<>, Target>>::first;

	template <size_t index>                           using erase_by_index = typename for_range<begin, end, erase_by_index_f, types_pack<type_list<>, meta_size_t<index>, meta_size_t<0>>>::first;

	template <template <typename> class Predicate>    using erase_if = typename for_range<begin, end, erase_if_f, types_pack<type_list<>, function_warpper<Predicate>>>::first;

	template <typename NewList, typename... NewLists> using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <size_t index>                           using get      = typename get_impl<index>::result;

	template <size_t index, typename NewType>         using set      = typename set_impl<0, index, NewType, head, type_list<>>::result;

	// template <typename Iterator, typename NewType>    using set_by_itr = for_range<begin, end, set_by_itr_f, types_pack<> >

	template <typename Target>                        using contains = meta_bool<std::is_same<Target, Type>::value || (std::is_same<Target, Types>::value || ...)>;

	template <template <typename...> class Function>  using for_each = type_list<Function<Type>, Function<Types>...>;

	template <template <typename...> class Container> using cast     = Container<Type, Types...>;

	template <typename OldType, typename NewType>     using replace  = type_list< meta_if<std::is_same<OldType, Type>::value, NewType, Type>, meta_if<std::is_same<OldType, Types>::value, NewType, Types>... >;

	template <template <typename> class Predicate, typename NewType> using replace_if = type_list< meta_if< Predicate<Type>::value, NewType, Type>, meta_if< Predicate<Types>::value, NewType, Types>... >;

	template <template <typename> class Predicate>    using find_if  = typename for_range<begin, end, find_if_f, types_pack<head, function_warpper<Predicate>>>::first;

	template <typename Target>                        using find     = find_if<find_pred<Target>::template f>;

	template <typename Separator>                     using split    = typename for_range<begin, end, split_f, types_pack<type_list<type_list<>>, Separator>>::first;

	template <typename any_type = meta_null>          using reverse  = typename reverse_impl<any_type>::first;
	
	//slice range: [from, to)
	//supports negative index like -1, but need to cast to size_t type before using: size_t(-1)
	template <size_t from, size_t to = length>        using slice    = meta_if<(from < to), 
			typename pop<(to > length ? size_t(-1) - to + 1 : length - to)>::
			       shift<(from > length ? length - size_t(-1) + from - 1 : from)>,
			type_list<>
		>;

	using front    = Type;

	using back     = get<length - 1>;


	// template <typename ElementType> 
	// static constexpr ElementType to_array[length] = {static_cast<ElementType>(Type{}), static_cast<ElementType>(Types{})...};

};

template <auto... values>
struct value_node {
	using has_next = meta_bool<false>;
};

template <auto this_value, auto... values>
struct value_node<this_value, values...> {
	using value_t = decltype(this_value);
	using next = value_node<values...>;
	using has_next = meta_bool<sizeof...(values) != 0>;

	static constexpr value_t value = this_value; 
};

template <auto... values>
struct value_list {
	//sizeof...(values) == 0;

	using head = value_node<>;

	static constexpr size_t length = 0;

	static constexpr list_category category = list_category::value_list;

private:
	template <auto...>
	friend class value_list;

	using self = value_list;

	template <typename...>
	struct concat_impl {
		using result = self;
	};
	template <auto... new_values, typename... NewLists>
	struct concat_impl<value_list<new_values...>, NewLists...> {
		using result = typename value_list<new_values...>::template concat_impl<NewLists...>::result;
	};

public:
	using begin    = value_node<>;

	using end      = value_node<>;

	using value_types = type_list<>;

	template <typename any_type = meta_null>          using reverse   = self;

	template <size_t shift_count = 1>                 using shift     = self;

	template <size_t pop_count = 1>                   using pop       = self;

	template <auto... new_values>                     using push      = value_list<new_values...>;
 
	template <auto... new_values>                     using unshift   = push<new_values...>;
	
	template <auto target>                            using erase     = self;

	template <size_t index>                           using erase_by_index = self;

	template <template <auto> class Predicate>        using erase_if  = self;

	template <size_t index, auto... new_values>       using insert    = push<new_values...>;

	template <typename NewList, typename... NewLists> using concat    = typename concat_impl<NewList, NewLists...>::result;

	template <auto target>                            using contains  = meta_bool<false>;

	template <template <auto> class Predicate>        using find_if   = value_node<>;

	template <auto target>                            using find      = value_node<>;

	template <template <auto...> class Function>      using for_each  = self;

	template <template <auto...> class Container>     using cast      = Container<>;

	template <auto old_value, auto new_value>         using replace   = self;

	template <template <auto> class Predicate, auto new_value> using replace_if = self;

	template <auto separator>                         using split     = self;

	template <size_t from, size_t to = length>        using slice     = self;

	template <typename FunctionT>
	static constexpr auto apply_to(FunctionT&& f) {return f(); }

};

template <auto value, auto... values>
struct value_list<value, values...> {

	using head = value_node<value, values...>;

	static constexpr size_t length = sizeof...(values) + 1;

	static constexpr list_category category = list_category::value_list;

private:
	template <auto...>
	friend class value_list;

	using self = value_list;

	template <auto a, auto b>
	//different types are always not equal
	struct value_equals: meta_bool<false> {};
	template <typename T, T a, T b>
	struct value_equals<a, b>: meta_bool<a == b> {};

	template <typename Node>
	struct to_list {}; 
	template <auto... node_values> 
	struct to_list<value_node<node_values...>> {
		using result = value_list<node_values...>;
	};

	template <size_t current_index, size_t index, bool insert_on_tail, typename CurrentNode, typename ResultList, auto... new_values>
	struct insert_impl {
		//insert_on_tail == false
		using result = typename insert_impl<current_index + 1, index, false, typename CurrentNode::next, typename ResultList::template push<CurrentNode::value>, new_values... >::result;
	};
	template <size_t index, typename CurrentNode, typename ResultList, auto... new_values>
	struct insert_impl<index, index, false, CurrentNode, ResultList, new_values...> {
		using result = typename ResultList::template push<new_values...>::template concat< to_list<CurrentNode> >;
	};
	//index >= length
	template <size_t current_index, size_t index, typename CurrentNode, typename ResultList, auto... new_values>
	struct insert_impl<current_index, index, true, CurrentNode, ResultList, new_values...> {
		using result = value_list<value, values..., new_values...>;
	};

	template <typename...>
	struct concat_impl {
		using result = self;
	};
	template <auto... new_values, typename... NewLists>
	struct concat_impl<value_list<new_values...>, NewLists...> {
		using result = typename value_list<value, values..., new_values...>::template concat_impl<NewLists...>::result;
	};

	template <size_t current_index, size_t index, auto new_value, typename CurrentNode, typename ResultList>
	struct set_impl {
		static_assert(index < length, "target index out of bound");
		using result = typename set_impl<current_index + 1, index, new_value, typename CurrentNode::next, typename ResultList::template push<typename CurrentNode::value> >::result;
	};
	template <size_t index, auto new_value, typename CurrentNode, typename ResultList>
	struct set_impl<index, index, new_value, CurrentNode, ResultList> {
		using result = typename ResultList::template push<new_value>::template concat< to_list<typename CurrentNode::next> >;
	};

	template <size_t pop_count, typename CurrentList, auto... current_values>
	struct pop_impl {
		using result = CurrentList;
	};
	template <size_t pop_count, typename CurrentList, auto current_value, auto... current_values>
	struct pop_impl<pop_count, CurrentList, current_value, current_values...> {
		using result = typename meta_if<pop_count == 0, 
			self
		>::template elif< sizeof...(current_values) == pop_count,
			typename CurrentList::template push<current_value>, 
			typename pop_impl<pop_count, typename CurrentList::template push<current_value>, current_values...>::result
		>;
	};

	template <auto i, typename CurrentList, typename PredicateWarpper> 
	struct erase_if_f: types_pack<meta_if<PredicateWarpper::template apply<i>::value, CurrentList, typename CurrentList::template push<i> >, PredicateWarpper> {};

	template <auto i, typename CurrentList, typename Index, typename CurrentIndex>
	struct erase_by_index_f: types_pack<meta_if<Index::value == CurrentIndex::value, CurrentList, typename CurrentList::template push<i>>, Index, typename CurrentIndex::inc> {};

	template <auto i, typename CurrentList, typename Target>
	struct erase_f: types_pack<meta_if<i == Target::value, CurrentList, typename CurrentList::template push<i>>, Target> {};

	template <auto i, typename ResultList, typename SeparatorWarpper>
	struct split_f: types_pack<meta_if<value_equals<i, SeparatorWarpper::value>::value, 
		typename ResultList::push<value_list<>>, 
		typename ResultList::set<ResultList::length - 1, typename ResultList::back::template push<i> > 
	>, SeparatorWarpper> {};

	template <auto i, typename ResultList>
	struct reverse_f: types_pack<typename ResultList::unshift<i>> {};
	
	template <typename>
	struct reverse_impl: for_value_range<head, value_node<>, reverse_f, types_pack<value_list<>>>{};
	
	template <auto i, typename CurrentNode, typename PredicateWarpper>
	struct find_if_f: meta_if<PredicateWarpper::template apply<i>::value, break_loop<types_pack<CurrentNode, PredicateWarpper>>, types_pack<typename CurrentNode::next, PredicateWarpper> > {};


	template <auto this_value, auto target, auto new_value>
	struct replace_f: meta_if<value_equals<target, this_value>::value, meta<new_value>, meta<this_value>> {};

	template <auto target>
	struct find_pred {
		template <auto i>
		struct f: value_equals<target, i> {};
	};


public:	


	using begin    = head;

	using end      = value_node<>;

	using value_types = type_list<decltype(value), decltype(values)...>;

	template <typename any_type = meta_null>            using reverse  = typename reverse_impl<any_type>::first;

	template <size_t shift_count = 1>                   using shift    = meta_if<shift_count == 0, self, typename value_list<values...>::shift<shift_count - 1>>;

	template <size_t pop_count = 1>                     using pop      = typename pop_impl<pop_count, value_list<>, value, values...>::result;

	template <auto... new_values>                       using push     = value_list<value, values..., new_values...>;
 
	template <auto... new_values>                       using unshift  = value_list<new_values..., value, values...>;

	template <auto target>                              using erase    = typename for_value_range<begin, end, erase_f, types_pack<value_list<>, meta<target>>>::first;

	template <size_t index>                             using erase_by_index = typename for_value_range<begin, end, erase_by_index_f, types_pack<value_list<>, meta_size_t<index>, meta_size_t<0>>>::first;

	template <template <auto> class Predicate>          using erase_if = typename for_value_range<begin, end, erase_if_f, types_pack<value_list<>, typename nontype<1>::function_warpper<Predicate>>>::first;

	template <size_t index, auto... new_values>         using insert   = typename insert_impl<0, index, (index >= length), head, value_list<>, new_values...>::result;

	template <typename NewList, typename... NewLists>   using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <size_t index>                             using get_warp = type_list<meta<value>, meta<values>...>::get<index>;
	template <size_t index> static constexpr auto             get      = get_warp<index>::value;
	
	template <size_t index, auto new_value>             using set      = typename set_impl<0, index, new_value, head, value_list<>>::result;

	template <auto target>                              using contains = meta_bool<value_equals<target, value>::value || (value_equals<target, values>::value || ...)>;

	template <template <auto> class Predicate>          using find_if  = typename for_value_range<begin, end, find_if_f, types_pack<head, typename nontype<1>::function_warpper<Predicate>>>::first;

	template <auto target>                              using find     = find_if<find_pred<target>::template f>;

	//requires Function<val>::value
	template <template <auto> class Function>           using for_each = value_list<Function<value>::value, Function<values>::value...>;

	template <template <auto...> class Container>       using cast     = Container<value, values...>;

	template <auto old_value, auto new_value>           using replace  = value_list<replace_f<value, old_value, new_value>::value, replace_f<values, old_value, new_value>::value...>

	template <template <auto> class Predicate, auto new_value> using replace_if = value_list<meta_if<Predicate<value>::value, meta<new_value>, meta<value>>::value, meta_if<Predicate<values>::value, meta<new_value>, meta<values>>::value... >;

	template <auto separator>                           using split    = typename for_value_range<begin, end, split_f, types_pack<type_list<value_list<>>, meta<separator>>>::first;

	//slice range: [from, to)
	//supports negative index like -1, but need to cast to size_t type before using: size_t(-1)
	template <size_t from, size_t to = length>          using slice    = meta_if<(from < to), 
			typename pop<(to > length ? size_t(-1) - to + 1 : length - to)>::
			       shift<(from > length ? length - size_t(-1) + from - 1 : from)>,
			value_list<>
		>;

	static constexpr auto front = value;

	static constexpr auto back  = type_list<meta<value>, meta<values>...>::back::value;

	template <typename FunctionT>
	static constexpr auto apply_to(FunctionT&& f) {return f(value, values...); }

};

template <typename T, T value, T... values>
struct value_list<value, values...> {

	using head = value_node<value, values...>;

	static constexpr size_t length = sizeof...(values) + 1;

	static constexpr T array[length] = {value, values...};

	static constexpr list_category category = list_category::value_list;
private:
	template <auto...>
	friend class value_list;

	using self = value_list;

	template <typename Node>
	struct to_list {}; 
	template <auto... node_values> 
	struct to_list<value_node<node_values...>> {
		using result = value_list<node_values...>;
	};

	template <size_t current_index, size_t index, bool insert_on_tail, typename CurrentNode, typename ResultList, auto... new_values>
	struct insert_impl {
		//insert_on_tail == false
		using result = typename insert_impl<current_index + 1, index, false, typename CurrentNode::next, typename ResultList::template push<CurrentNode::value>, new_values... >::result;
	};
	template <size_t index, typename CurrentNode, typename ResultList, auto... new_values>
	struct insert_impl<index, index, false, CurrentNode, ResultList, new_values...> {
		using result = typename ResultList::template push<new_values...>::template concat< to_list<CurrentNode> >;
	};
	//index >= length
	template <size_t current_index, size_t index, typename CurrentNode, typename ResultList, auto... new_values>
	struct insert_impl<current_index, index, true, CurrentNode, ResultList, new_values...> {
		using result = value_list<value, values..., new_values...>;
	};

	template <typename...>
	struct concat_impl {
		using result = self;
	};
	template <auto... new_values, typename... NewLists>
	struct concat_impl<value_list<new_values...>, NewLists...> {
		using result = typename value_list<value, values..., new_values...>::template concat_impl<NewLists...>::result;
	};

	template <size_t current_index, size_t index, auto new_value, typename CurrentNode, typename ResultList>
	struct set_impl {
		static_assert(index < length, "target index out of bound");
		using result = typename set_impl<current_index + 1, index, new_value, typename CurrentNode::next, typename ResultList::template push<typename CurrentNode::value> >::result;
	};
	template <size_t index, auto new_value, typename CurrentNode, typename ResultList>
	struct set_impl<index, index, new_value, CurrentNode, ResultList> {
		using result = typename ResultList::template push<new_value>::template concat< to_list<typename CurrentNode::next> >;
	};

	template <size_t pop_count, typename CurrentList, auto... current_values>
	struct pop_impl {
		using result = CurrentList;
	};
	template <size_t pop_count, typename CurrentList, auto current_value, auto... current_values>
	struct pop_impl<pop_count, CurrentList, current_value, current_values...> {
		using result = typename meta_if<pop_count == 0, 
			self
		>::template elif< sizeof...(current_values) == pop_count,
			typename CurrentList::template push<current_value>, 
			typename pop_impl<pop_count, typename CurrentList::template push<current_value>, current_values...>::result
		>;
	};

	template <auto i, typename CurrentList, typename PredicateWarpper> 
	struct erase_if_f: types_pack<meta_if<PredicateWarpper::template apply<i>::value, CurrentList, typename CurrentList::template push<i> >, PredicateWarpper> {};

	template <auto i, typename CurrentList, typename Index, typename CurrentIndex>
	struct erase_by_index_f: types_pack<meta_if<Index::value == CurrentIndex::value, CurrentList, typename CurrentList::template push<i>>, Index, typename CurrentIndex::inc> {};

	template <auto i, typename CurrentList, typename Target>
	struct erase_f: types_pack<meta_if<i == Target::value, CurrentList, typename CurrentList::template push<i>>, Target> {};

	template <auto i, typename ResultList, typename SeparatorWarpper>
	struct split_f: types_pack<meta_if<i == SeparatorWarpper::value, 
		typename ResultList::push<value_list<>>, 
		typename ResultList::set<ResultList::length - 1, typename ResultList::back::template push<i> > 
	>, SeparatorWarpper> {};

	template <auto i, typename ResultList>
	struct reverse_f: types_pack<typename ResultList::unshift<i>> {};
	
	template <typename>
	struct reverse_impl: for_value_range<head, value_node<>, reverse_f, types_pack<value_list<>>>{};
	
	template <auto i, typename CurrentNode, typename PredicateWarpper>
	struct find_if_f: meta_if<PredicateWarpper::template apply<i>::value, break_loop<types_pack<CurrentNode, PredicateWarpper>>, types_pack<typename CurrentNode::next, PredicateWarpper> > {};


	template <auto this_value, auto target, auto new_value>
	struct replace_f: meta_if<target == this_value, meta<new_value>, meta<this_value>> {};

	template <auto target>
	struct find_pred {
		template <auto i>
		struct f: meta_bool<target == i> {};
	};


public:	


	using begin    = head;

	using end      = value_node<>;

	using value_types = type_list<T>;
	using value_t  = T;

	template <typename any_type = meta_null>            using reverse  = typename reverse_impl<any_type>::first;

	template <size_t shift_count = 1>                   using shift    = meta_if<shift_count == 0, self, typename value_list<values...>::shift<shift_count - 1>>;

	template <size_t pop_count = 1>                     using pop      = typename pop_impl<pop_count, value_list<>, value, values...>::result;

	template <auto... new_values>                       using push     = value_list<value, values..., new_values...>;
 
	template <auto... new_values>                       using unshift  = value_list<new_values..., value, values...>;

	template <auto target>                              using erase    = typename for_value_range<begin, end, erase_f, types_pack<value_list<>, meta<target>>>::first;

	template <size_t index>                             using erase_by_index = typename for_value_range<begin, end, erase_by_index_f, types_pack<value_list<>, meta_size_t<index>, meta_size_t<0>>>::first;

	template <template <auto> class Predicate>          using erase_if = typename for_value_range<begin, end, erase_if_f, types_pack<value_list<>, typename nontype<1>::function_warpper<Predicate>>>::first;

	template <size_t index, auto... new_values>         using insert   = typename insert_impl<0, index, (index >= length), head, value_list<>, new_values...>::result;

	template <typename NewList, typename... NewLists>   using concat   = typename concat_impl<NewList, NewLists...>::result;

	template <size_t index>                             using get_warp = meta<array[index]>;
	template <size_t index> static constexpr auto             get      = array[index]; 
	static constexpr auto at(size_t index) noexcept{ return array[index % length]; }  
	
	template <size_t index, auto new_value>             using set      = typename set_impl<0, index, new_value, head, value_list<>>::result;

	template <auto target>                              using contains = meta_bool<(target == value) || ((target == values) || ...)>;

	template <template <auto> class Predicate>          using find_if  = typename for_value_range<begin, end, find_if_f, types_pack<head, typename nontype<1>::function_warpper<Predicate>>>::first;

	template <auto target>                              using find     = find_if<find_pred<target>::template f>;

	//requires Function<val>::value
	template <template <auto> class Function>           using for_each = value_list<Function<value>::value, Function<values>::value...>;

	template <template <auto...> class Container>       using cast     = Container<value, values...>;

	template <auto old_value, auto new_value>           using replace  = value_list<replace_f<value, old_value, new_value>::value, replace_f<values, old_value, new_value>::value...>

	template <template <auto> class Predicate, auto new_value> using replace_if = value_list<meta_if<Predicate<value>::value, meta<new_value>, meta<value>>::value, meta_if<Predicate<values>::value, meta<new_value>, meta<values>>::value... >;

	template <auto separator>                           using split    = typename for_value_range<begin, end, split_f, types_pack<type_list<value_list<>>, meta<separator>>>::first;

	//slice range: [from, to)
	//supports negative index like -1, but need to cast to size_t type before using: size_t(-1)
	template <size_t from, size_t to = length>          using slice    = meta_if<(from < to), 
			typename pop<(to > length ? size_t(-1) - to + 1 : length - to)>::
			       shift<(from > length ? length - size_t(-1) + from - 1 : from)>,
			value_list<>
		>;

	static constexpr auto front = value;

	static constexpr auto back  = array[length - 1];

	template <typename FunctionT>
	static constexpr auto apply_to(FunctionT&& f) {return f(value, values...); }

};

template <typename CharT, CharT... chars>
struct meta_string: value_list<chars..., CharT('\0')> {

};

namespace op {

namespace detail {


template <typename List, list_category category, typename... Args>
struct push_impl {};
template <typename List, list_category category, typename... Args>
struct push_impl<List, list_category::type_list, Args...> {
	using result = typename List::push<Args...>;
};
template <typename List, list_category category, typename... Args>
struct push_impl<List, list_category::value_list, Args...> {
	using result = typename List::push<Args::value...>;
};

template <typename List, list_category category, typename... Args>
struct unshift_impl {};
template <typename List, list_category category, typename... Args>
struct unshift_impl<List, list_category::type_list, Args...> {
	using result = typename List::unshift<Args...>;
};
template <typename List, list_category category, typename... Args>
struct unshift_impl<List, list_category::value_list, Args...> {
	using result = typename List::unshift<Args::value...>;
};

template <typename List, list_category category, typename Index, typename... Args>
struct insert_impl {};
template <typename List, list_category category, typename Index, typename... Args>
struct insert_impl<List, list_category::type_list, Index, Args...> {
	using result = typename List::insert<Index::value, Args...>;
};
template <typename List, list_category category, typename Index, typename... Args>
struct insert_impl<List, list_category::value_list, Index, Args...> {
	using result = typename List::insert<Index::value, Args::value...>;
};

template <typename List, list_category category, typename Index, typename Arg>
struct set_impl {};
template <typename List, list_category category, typename Index, typename Arg>
struct set_impl<List, list_category::type_list, Index, Arg> {
	using result = typename List::set<Index::value, Arg>;
};
template <typename List, list_category category, typename Index, typename Arg>
struct set_impl<List, list_category::value_list, Index, Arg> {
	using result = typename List::set<Index::value, Arg::value>;
};

template <typename List, list_category category, typename Index>
struct get_impl {};
template <typename List, list_category category, typename Index>
struct get_impl<List, list_category::type_list, Index> {
	using result = typename List::get<Index::value>;
};
template <typename List, list_category category, typename Index>
struct get_impl<List, list_category::value_list, Index> {
	using result = typename List::get_warp<Index::value>;
};

template <typename List, list_category category, typename Element>
struct erase_impl {};
template <typename List, list_category category, typename Element>
struct erase_impl<List, list_category::type_list, Element> {
	using result = typename List::erase<Element>;
};
template <typename List, list_category category, typename Element>
struct erase_impl<List, list_category::value_list, Element> {
	using result = typename List::erase<Element::value>;
};

template <typename List, list_category category, typename Element>
struct find_impl {};
template <typename List, list_category category, typename Element>
struct find_impl<List, list_category::type_list, Element> {
	using result = typename List::find<Element>;
};
template <typename List, list_category category, typename Element>
struct find_impl<List, list_category::value_list, Element> {
	using result = typename List::find<Element::value>;
};

template <typename List, list_category category, typename Element>
struct split_impl {};
template <typename List, list_category category, typename Element>
struct split_impl<List, list_category::type_list, Element> {
	using result = typename List::split<Element>;
};
template <typename List, list_category category, typename Element>
struct split_impl<List, list_category::value_list, Element> {
	using result = typename List::split<Element::value>;
};

template <typename List, list_category category, typename OldElement, typename NewElement>
struct replace_impl {};
template <typename List, list_category category, typename OldElement, typename NewElement>
struct replace_impl<List, list_category::type_list, OldElement, NewElement> {
	using result = typename List::replace<OldElement, NewElement>;
};
template <typename List, list_category category, typename OldElement, typename NewElement>
struct replace_impl<List, list_category::value_list, OldElement, NewElement> {
	using result = typename List::replace<OldElement::value, NewElement::value>;
};

template <typename List, list_category category, typename Predicate, typename NewElement>
struct replace_if_impl {};
template <typename List, list_category category, typename Predicate, typename NewElement>
struct replace_if_impl<List, list_category::type_list, Predicate, NewElement> {
	using result = typename List::replace_if<typename Predicate::template eval, NewElement>;
};
template <typename List, list_category category, typename Predicate, typename NewElement>
struct replace_if_impl<List, list_category::value_list, Predicate, NewElement> {
	using result = typename List::replace_if<typename Predicate::template eval, NewElement::value>;
};

} //namespace helper_detail


template <typename List> using begin    = typename List::begin;
template <typename List> using end      = typename List::end;
template <typename List> using reverse  = typename List::template reverse<>;
template <typename List, typename ShiftCount> using shift          = typename List::template shift<ShiftCount::value>;
template <typename List, typename PopCount>   using pop            = typename List::template pop<PopCount::value>;
template <typename List, typename Index>      using erase_by_index = typename List::template erase_by_index<Index::value>;
template <typename List, typename Predicate>  using erase_if       = typename List::template erase_if<Predicate::template eval>;
template <typename List, typename... Lists>   using concat         = typename List::template concat<Lists...>;
template <typename List, typename Predicate>  using find_if        = typename List::template find_if<Predicate::template eval>;
template <typename List, typename Function>   using for_each       = typename List::template for_each<Function::template eval>;
template <typename List, typename Container>  using cast           = typename List::template cast<Container::template eval>;
template <typename List, typename From, typename To>  using slice  = typename List::template slice<From::value, To::value>;


template <typename List, typename... NewElements> using push    = typename detail::   push_impl<List, List::category, NewElements...>::result;
template <typename List, typename... NewElements> using unshift = typename detail::unshift_impl<List, List::category, NewElements...>::result;
template <typename List, typename Index>          using get     = typename detail::    get_impl<List, List::category, Index>::result;
template <typename List, typename Element>        using erase   = typename detail::  erase_impl<List, List::category, Element>::result;
template <typename List, typename Element>        using find    = typename detail::   find_impl<List, List::category, Element>::result;
template <typename List, typename Element>        using split   = typename detail::  split_impl<List, List::category, Element>::result;
template <typename List, typename OldElement, typename NewElement> using replace    = typename detail::replace_impl<List, List::category, OldElement, NewElement>::result;
template <typename List, typename Predicate, typename NewElement>  using replace_if = typename detail::replace_if_impl<List, List::category, Predicate, NewElement>::result;
template <typename List, typename Index, typename NewValue>        using set        = typename detail::set_impl<List, List::category, Index, NewValue>::result;
template <typename List, typename Index, typename... NewElements>  using insert     = typename detail::insert_impl<List, List::category, List::category, Index, NewElements...>::result;

} //namespace op



} // namespace meta
} // namespace rais

#endif //RAIS_META_LIST_HPP

