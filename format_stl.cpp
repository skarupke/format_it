#include "format_stl.hpp"

#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
#include <vector>
#include <set>
#include <map>
#include <deque>
#include <list>
#include <forward_list>
#include <array>
#include <unordered_set>
#include <unordered_map>
namespace
{
TEST(format_stl, tuple)
{
	std::tuple<int, int, float> a{ 1, 2, 3.0f };
	fmt::stack_print<1024> print(a);
	ASSERT_EQ("{ 1, 2, 3 }", print);
	std::tuple<> empty;
	fmt::stack_print<1024> print_empty(empty);
	ASSERT_EQ("{ }", print_empty);
}
template<typename T>
void TestSortedContainer()
{
	T a = { 1, 2, 3, 4, 5 };
	fmt::stack_print<1024> print(a);
	ASSERT_EQ("{ 1, 2, 3, 4, 5 }", print);
	a.clear();
	fmt::stack_print<1024> print_again(a);
	ASSERT_EQ("{ }", print_again);
}
TEST(format_stl, initializer_list)
{
	std::initializer_list<int> il = { 1, 2, 3, 4, 5 };
	fmt::stack_print<1024> print(il);
	ASSERT_EQ("{ 1, 2, 3, 4, 5 }", print);
	std::initializer_list<int> empty = { };
	fmt::stack_print<1024> print_empty(empty);
	ASSERT_EQ("{ }", print_empty);
}
TEST(format_stl, vector)
{
	TestSortedContainer<std::vector<int> >();
}
TEST(format_stl, deque)
{
	TestSortedContainer<std::deque<int> >();
}
TEST(format_stl, list)
{
	TestSortedContainer<std::list<int> >();
}
TEST(format_stl, forward_list)
{
	TestSortedContainer<std::forward_list<int> >();
}
TEST(format_stl, array)
{
	std::array<int, 5> a = { { 1, 2, 3, 4, 5 } };
	fmt::stack_print<1024> print(a);
	ASSERT_EQ("{ 1, 2, 3, 4, 5 }", print);
}
TEST(format_stl, set)
{
	TestSortedContainer<std::set<int> >();
	std::multiset<int> b = { 1, 2, 2, 3 };
	fmt::stack_print<1024> print_b(b);
	ASSERT_EQ("{ 1, 2, 2, 3 }", print_b);
	b.clear();
	fmt::stack_print<1024> print_empty(b);
	ASSERT_EQ("{ }", print_empty);
}
TEST(format_stl, map)
{
	std::map<int, int> a = { { 1, 2 }, { 2, 3 }, { 3, 4 } };
	fmt::stack_print<1024> print(a);
	ASSERT_EQ("{ { 1, 2 }, { 2, 3 }, { 3, 4 } }", print);
	a.clear();
	fmt::stack_print<1024> print_a_empty(a);
	ASSERT_EQ("{ }", print_a_empty);
	std::multimap<int, int> b = { { 1, 2 }, { 1, 2 }, { 2, 3 }, { 3, 4 } };
	fmt::stack_print<1024> print_b(b);
	ASSERT_EQ("{ { 1, 2 }, { 1, 2 }, { 2, 3 }, { 3, 4 } }", print_b);
	b.clear();
	fmt::stack_print<1024> print_b_empty(b);
	ASSERT_EQ("{ }", print_b_empty);
}
TEST(format_stl, unordered_set)
{
	std::unordered_set<int> a = { 1, 2, 3 };
	fmt::stack_print<1024>{a};
	a.clear();
	fmt::stack_print<1024> a_empty(a);
	ASSERT_EQ("{ }", a_empty);
	std::unordered_multiset<int> b = { 1, 2, 2, 3 };
	fmt::stack_print<1024>{b};
	b.clear();
	fmt::stack_print<1024> b_empty(b);
	ASSERT_EQ("{ }", b_empty);
}
TEST(format_stl, unordered_map)
{
	std::unordered_map<int, int> a = { { 1, 2 }, { 2, 3 }, { 3, 4 } };
	fmt::stack_print<1024>{a};
	a.clear();
	fmt::stack_print<1024> a_empty(a);
	ASSERT_EQ("{ }", a_empty);
	std::unordered_multimap<int, int> b = { { 1, 2 }, { 1, 2 }, { 2, 3 }, { 3, 4 } };
	fmt::stack_print<1024>{b};
	b.clear();
	fmt::stack_print<1024> b_empty(b);
	ASSERT_EQ("{ }", b_empty);
}
}
#endif
