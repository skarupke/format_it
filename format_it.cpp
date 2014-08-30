#include "format_it.hpp"

static const char * reason_to_message(fmt::format_error::Reason reason)
{
	switch(reason)
	{
	case fmt::format_error::UnusedArgument:
		return "Not all arguments were used in the format string";
	case fmt::format_error::PercentNotFollowedByNumber:
		return "A percent sign '%' was not followed by a number. If you intended to print a percent sign use two percents %%";
	case fmt::format_error::OpenPercentAtEndOfInput:
		return "A percent sign '%' was used at the end of the sequence without a number following it. If you intended to print a percent sign use two percents %%";
	case fmt::format_error::FormatIndexOutOfRange:
		return "Format index out of range";
	}
	// forgot to handle a case in the switch statement above;
	return "Unknown reason";
}

fmt::format_error::format_error(Reason reason)
	: std::runtime_error(reason_to_message(reason)), _reason(reason)
{
}


#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
namespace
{
TEST(format_it, format_to_string)
{
	std::string test;
	fmt::make_format_it<char>(std::back_inserter(test)).format("[%0, %1, %2]", "Hello", "World", 5);
	ASSERT_EQ("[Hello, World, 5]", test);
}
TEST(format_it, wchar)
{
	std::wstring test;
	fmt::make_format_it<wchar_t>(std::back_inserter(test)).format(L"%0: %1, %2, %3", 5, L"bar", "baz", 0.7);
	ASSERT_EQ(L"5: bar, baz, 0.7", test);
}
TEST(format_it, utf32)
{
	std::u32string test;
	char bytes[4] = "foo";
	fmt::make_format_it<char32_t>(std::back_inserter(test)).format(U"%0: %1, %2, %3, %4", 5, U"bar", "baz", 0.7, bytes);
	ASSERT_EQ(U"5: bar, baz, 0.7, foo", test);
}
TEST(format_it, percent)
{
	std::string foo;
	fmt::make_format_it<char>(std::back_inserter(foo)).format("%0%%", 10);
	ASSERT_EQ("10%", foo);
}
TEST(format_it, order_independent)
{
	std::string foo;
	fmt::make_format_it<char>(std::back_inserter(foo)).format("%1%0%0%1", "foo", std::string("bar"));
	ASSERT_EQ("barfoofoobar", foo);
}
TEST(format_it, out_of_range)
{
	bool did_throw = false;
	try
	{
		std::string foo;
		fmt::make_format_it<char>(std::back_inserter(foo)).format("%0%1", 1.0f);
	}
	catch(const fmt::format_error & error)
	{
		did_throw = true;
		ASSERT_EQ(fmt::format_error::FormatIndexOutOfRange, error.reason());
	}
	ASSERT_TRUE(did_throw);
}
TEST(format_it, invalid_format_string)
{
	bool did_throw = false;
	const char format[] = "%%%0";
	try
	{
		std::string foo;
		fmt::make_format_it<char>(std::back_inserter(foo)).format(format + 0, format + 3);
	}
	catch(const fmt::format_error & error)
	{
		did_throw = true;
		ASSERT_EQ(fmt::format_error::OpenPercentAtEndOfInput, error.reason());
	}
	ASSERT_TRUE(did_throw);
}
TEST(format_it, print_separator)
{
	std::string print;
	fmt::make_format_it<char>(std::back_inserter(print)).print("Hello","World","!");
	ASSERT_EQ("Hello World !", print);
	std::string print_comma;
	fmt::make_format_it<char>(std::back_inserter(print_comma)).print<','>("Hello","World","!");
	ASSERT_EQ("Hello,World,!", print_comma);
	std::string printpacked;
	fmt::make_format_it<char>(std::back_inserter(printpacked)).printpacked("Hello","World","!");
	ASSERT_EQ("HelloWorld!", printpacked);
	std::string print_separated;
	fmt::make_format_it<char>(std::back_inserter(print_separated)).print_separated(", ", "Hello","World","!");
	ASSERT_EQ("Hello, World, !", print_separated);
}
}
#endif
