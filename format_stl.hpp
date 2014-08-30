#pragma once

#include "stack_format.hpp"
#include "format_helpers.hpp"

namespace std
{
template<typename F, typename S>
struct pair;
template<typename... Ts>
class tuple;
template<typename K, typename V, typename C, typename A>
class map;
template<typename K, typename V, typename C, typename A>
class multimap;
template<typename T, typename C, typename A>
class set;
template<typename T, typename C, typename A>
class multiset;
template<typename K, typename V, typename H, typename E, typename A>
class unordered_map;
template<typename K, typename V, typename H, typename E, typename A>
class unordered_multimap;
template<typename T, typename H, typename E, typename A>
class unordered_set;
template<typename T, typename H, typename E, typename A>
class unordered_multiset;
template<typename T, size_t N>
struct array;
template<typename T, typename A>
class vector;
template<typename T, typename A>
class deque;
template<typename T, typename A>
class list;
template<typename T, typename A>
class forward_list;
}

namespace fmt
{
template<typename C, typename It, typename F, typename S>
format_it<C, It> format(format_it<C, It> it, const std::pair<F, S> & value)
{
	return it.printpacked("{ ", value.first, ", ", value.second, " }");
}
namespace detail
{
template<size_t N>
struct TupleFormatter
{
	template<typename C, typename It, typename... Ts, typename... Us>
	format_it<C, It> operator()(format_it<C, It> it, const std::tuple<Ts...> & tuple, const Us &... values) const
	{
		return TupleFormatter<N - 1>()(it, tuple, std::get<N - 1>(tuple), values...);
	}
};
template<>
struct TupleFormatter<0>
{
	template<typename C, typename It, typename... Ts, typename... Us>
	format_it<C, It> operator()(format_it<C, It> it, const std::tuple<Ts...> &, const Us &... values) const
	{
		*it++ = "{ ";
		it.print_separated(", ", values...);
		*it++ = " }";
		return it;
	}
	template<typename C, typename It>
	format_it<C, It> operator()(format_it<C, It> it, const std::tuple<> &) const
	{
		return it.print("{ }");
	}
};
}
template<typename C, typename It, typename... Ts>
format_it<C, It> format(format_it<C, It> it, const std::tuple<Ts...> & value)
{
	return detail::TupleFormatter<sizeof...(Ts)>()(it, value);
}
namespace detail
{
template<typename C, typename It, typename T>
format_it<C, It> format_container(format_it<C, It> it, const T & value)
{
	if (value.empty()) return it.print("{ }");
	else return it.print('{', with_separator(value, ", "), '}');
}
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, const std::initializer_list<T> & value)
{
	if (value.size() == 0) return it.print("{ }");
	else return it.print('{', with_separator(value, ", "), '}');
}
template<typename C, typename It, typename K, typename V, typename Comp, typename A>
format_it<C, It> format(format_it<C, It> it, const std::map<K, V, Comp, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename K, typename V, typename Comp, typename A>
format_it<C, It> format(format_it<C, It> it, const std::multimap<K, V, Comp, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename Comp, typename A>
format_it<C, It> format(format_it<C, It> it, const std::set<T, Comp, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename Comp, typename A>
format_it<C, It> format(format_it<C, It> it, const std::multiset<T, Comp, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename K, typename V, typename H, typename E, typename A>
format_it<C, It> format(format_it<C, It> it, const std::unordered_map<K, V, H, E, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename K, typename V, typename H, typename E, typename A>
format_it<C, It> format(format_it<C, It> it, const std::unordered_multimap<K, V, H, E, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename H, typename E, typename A>
format_it<C, It> format(format_it<C, It> it, const std::unordered_set<T, H, E, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename H, typename E, typename A>
format_it<C, It> format(format_it<C, It> it, const std::unordered_multiset<T, H, E, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename A>
format_it<C, It> format(format_it<C, It> it, const std::vector<T, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename A>
format_it<C, It> format(format_it<C, It> it, const std::deque<T, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename A>
format_it<C, It> format(format_it<C, It> it, const std::list<T, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, typename A>
format_it<C, It> format(format_it<C, It> it, const std::forward_list<T, A> & value)
{
	return detail::format_container(it, value);
}
template<typename C, typename It, typename T, size_t N>
format_it<C, It> format(format_it<C, It> it, const std::array<T, N> & value)
{
	return it.print('{', with_separator(value, ", "), '}');
}
}
