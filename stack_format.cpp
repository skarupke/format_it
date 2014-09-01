/*
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.
In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
For more information, please refer to <http://unlicense.org/>
*/

#include "stack_format.hpp"

#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
namespace
{
TEST(stack_format, simple)
{
	fmt::stack_format<1024> hi("Hello, World!");
	ASSERT_EQ("Hello, World!", hi);
}
TEST(stack_format, two_strings)
{
	bool did_throw = false;
	try
	{
		fmt::stack_format<1024>("Hello", "World");
	}
	catch(const fmt::format_error & error)
	{
		did_throw = true;
		ASSERT_EQ(fmt::format_error::UnusedArgument, error.reason());
	}
	ASSERT_TRUE(did_throw);
}
TEST(stack_format, format)
{
	fmt::stack_format<1024> foo("%0, %0, %1", std::string("foo"), 1);
	ASSERT_EQ("foo, foo, 1", foo);
}
TEST(stack_format, string_format)
{
	fmt::stack_format<512> foo(std::string("%%%0"), "Hi");
	ASSERT_EQ("%Hi", foo);
}

TEST(stack_format, print)
{
	fmt::stack_print<1024> print("Hello,", 1);
	ASSERT_EQ("Hello, 1", print);
}
TEST(stack_format, iterators)
{
	fmt::stack_print<1024> foo(0, 1, 2, 3);
	ASSERT_TRUE(std::equal(foo.begin(), foo.end(), "0 1 2 3"));
}
TEST(stack_format, iterators_zero_sized)
{
	fmt::stack_print<0> foo(1, 2, 3);
	ASSERT_TRUE(std::equal(foo.begin(), foo.end(), "1 2 3"));
}
TEST(stack_format, multidigit_index)
{
	fmt::stack_format<1024> foo("%0%1%2%3%4%5%6%7%8%9%10%11%12%13%00%02", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
	ASSERT_EQ("01234567891011121302", foo);
}
TEST(stack_format, overflow)
{
	// this has to fall back to heap allocation
	fmt::stack_format<4> foo("%0, %1", 10000.0f, 66.0f);
	ASSERT_EQ("10000, 66", foo);
}
template<typename C = char>
struct scoped_change_callback
{
	scoped_change_callback(void (*callback)(const C *, const C *))
		: old_callback(fmt::format_overflow_callback<C>::callback)
	{
		fmt::format_overflow_callback<C>::callback = callback;
	}
	~scoped_change_callback()
	{
		fmt::format_overflow_callback<C>::callback = old_callback;
	}

	void (*old_callback)(const C *, const C *);
};

TEST(stack_format, overflow_callback)
{
	static bool did_trigger = false;
	scoped_change_callback<> callback([](const char * begin, const char * end)
	{
		did_trigger = true;
		ASSERT_TRUE(std::equal(begin, end, "foob"));
	});
	fmt::stack_format<5>("%0%1", "foo", "bar");
	ASSERT_TRUE(did_trigger);
}
TEST(stack_format, overflow_callback_dont_trigger)
{
	static bool did_trigger = false;
	scoped_change_callback<> callback([](const char * begin, const char * end)
	{
		did_trigger = true;
		ASSERT_TRUE(std::equal(begin, end, "foob"));
	});
	fmt::stack_format<10>("%0%1", "foo", "bar");
	ASSERT_FALSE(did_trigger);
}
TEST(stack_format, zero_sized)
{
	fmt::stack_format<0> foo("%0, %0, %1", std::string("foo"), 1);
	ASSERT_EQ("foo, foo, 1", foo);
}
TEST(stack_format, overflow_callback_zero_sized)
{
	static bool did_trigger = false;
	scoped_change_callback<> callback([](const char *, const char *)
	{
		did_trigger = true;
	});
	fmt::stack_format<0>("%0%1", "foo", "bar");
	ASSERT_FALSE(did_trigger);
}
static constexpr int profile_count = 2000000;
TEST(stack_format, DISABLED_profile)
{
	for (int i = 0; i < profile_count; ++i)
	{
		//fmt::stack_format<1024>("[%0, %1, %2]", i, i * 2, i / 2);
		fmt::stack_format<1024>("[%0, %1, %2]", static_cast<float>(i), i * 2.0f, i / 2.0f);
		//fmt::stack_format<1024>("[%0, %1, %2]", "foo", "bar", "baz");
	}
}
TEST(stack_format, DISABLED_profile_printf)
{
	for (int i = 0; i < profile_count; ++i)
	{
		char buffer[1024];
		//snprintf(buffer, 1024, "[%d, %d, %d]", i, i * 2, i / 2);
		snprintf(buffer, 1024, "[%g, %g, %g]", static_cast<float>(i), i * 2.0f, i / 2.0f);
		//snprintf(buffer, 1024, "[%s, %s, %s]", "foo", "bar", "baz");
	}
}
TEST(stack_format, DISABLED_profile_stringstream)
{
	for (int i = 0; i < profile_count; ++i)
	{
		//std::stringstream() << '[' << i << ", " << i * 2 << ", " << i / 2 << ']';
		std::stringstream() << '[' << static_cast<float>(i) << ", " << i * 2.0f << ", " << i / 2.0f << ']';
		//std::stringstream() << '[' << "foo" << ", " << "bar" << ", " << "baz" << ']';
	}
}
}
#endif
