#include "format_helpers.hpp"

#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
namespace
{
TEST(format_helpers, padding)
{
	ASSERT_EQ("   1", fmt::stack_print<1024>(fmt::pad_left(1, 4)));
	ASSERT_EQ("1   ", fmt::stack_print<1024>(fmt::pad_right(1, 4)));
	ASSERT_EQ("  1 ", fmt::stack_print<1024>(fmt::pad_both(1, 4)));
}
TEST(format_helpers, padding_overflow)
{
	ASSERT_EQ("12345", fmt::stack_print<1024>(fmt::pad_left(12345, 4)));
	ASSERT_EQ("12345", fmt::stack_print<1024>(fmt::pad_both(12345, 4)));
}
TEST(format_helpers, padint)
{
	ASSERT_EQ("0123", fmt::stack_print<1024>(fmt::pad_int(123, 4)));
	ASSERT_EQ("1234", fmt::stack_print<1024>(fmt::pad_int(1234, 4)));
	ASSERT_EQ("12345", fmt::stack_print<1024>(fmt::pad_int(12345, 4)));
	ASSERT_EQ("-012", fmt::stack_print<1024>(fmt::pad_int(-12, 4)));
	ASSERT_EQ("-123", fmt::stack_print<1024>(fmt::pad_int(-123, 4)));
	ASSERT_EQ("-1234", fmt::stack_print<1024>(fmt::pad_int(-1234, 4)));
}
TEST(format_helpers, separator)
{
	std::vector<int> a = { 1, 2, 3, 4, 5 };
	fmt::stack_print<1024> print(fmt::with_separator(a, ", "));
	ASSERT_EQ("1, 2, 3, 4, 5", print);
}
TEST(format_helpers, separator_it)
{
	std::vector<int> a = { 1, 2, 3, 4, 5 };
	std::string formatted;
	std::transform(a.begin(), a.end(), fmt::with_separator(fmt::make_format_it(std::back_inserter(formatted)), ", "), [](int a){ return a * 10; });
	ASSERT_EQ("10, 20, 30, 40, 50", formatted);
}

/*TEST(format_helpers, more_complex_padding)
{
	fmt::stack_print<1024> pad_left_single(fmt::pad_left(1, 4, "𝄞"));
	ASSERT_EQ("𝄞𝄞𝄞1", pad_left_single);
	fmt::stack_print<1024> pad_right_single(fmt::pad_right(1, 4, "𝄞"));
	ASSERT_EQ("1𝄞𝄞𝄞", pad_right_single);
	fmt::stack_print<1024> pad_middle_single(fmt::padmiddle(1, 4, "𝄞"));
	ASSERT_EQ("𝄞𝄞1𝄞", pad_middle_single);
	fmt::stack_print<1024> pad_left_double(fmt::pad_left(12, 4, "𝄞"));
	ASSERT_EQ("𝄞𝄞12", pad_left_double);
	fmt::stack_print<1024> pad_right_double(fmt::pad_right(12, 4, "𝄞"));
	ASSERT_EQ("12𝄞𝄞", pad_right_double);
	fmt::stack_print<1024> pad_middle_double(fmt::padmiddle(12, 4, "𝄞"));
	ASSERT_EQ("𝄞12𝄞", pad_middle_double);
	fmt::stack_print<1024> pad_left_triple(fmt::pad_left(123, 4, "𝄞"));
	ASSERT_EQ("𝄞123", pad_left_triple);
	fmt::stack_print<1024> pad_right_triple(fmt::pad_right(123, 4, "𝄞"));
	ASSERT_EQ("123𝄞", pad_right_triple);
	fmt::stack_print<1024> pad_middle_triple(fmt::padmiddle(123, 4, "𝄞"));
	ASSERT_EQ("𝄞123", pad_middle_triple);
}
TEST(format_helpers, more_complex_value)
{
	fmt::stack_print<1024> pad_left_single(fmt::pad_left("𝄞", 4));
	ASSERT_EQ("   𝄞", pad_left_single);
	fmt::stack_print<1024> pad_right_single(fmt::pad_right("𝄞", 4));
	ASSERT_EQ("𝄞   ", pad_right_single);
	fmt::stack_print<1024> pad_middle_single(fmt::padmiddle("𝄞", 4));
	ASSERT_EQ("  𝄞 ", pad_middle_single);
	fmt::stack_print<1024> pad_left_double(fmt::pad_left("𝄞𝄞", 4));
	ASSERT_EQ("  𝄞𝄞", pad_left_double);
	fmt::stack_print<1024> pad_right_double(fmt::pad_right("𝄞𝄞", 4));
	ASSERT_EQ("𝄞𝄞  ", pad_right_double);
	fmt::stack_print<1024> pad_middle_double(fmt::padmiddle("𝄞𝄞", 4));
	ASSERT_EQ(" 𝄞𝄞 ", pad_middle_double);
	fmt::stack_print<1024> pad_left_triple(fmt::pad_left("𝄞𝄞𝄞", 4));
	ASSERT_EQ(" 𝄞𝄞𝄞", pad_left_triple);
	fmt::stack_print<1024> pad_right_triple(fmt::pad_right("𝄞𝄞𝄞", 4));
	ASSERT_EQ("𝄞𝄞𝄞 ", pad_right_triple);
	fmt::stack_print<1024> pad_middle_triple(fmt::padmiddle("𝄞𝄞𝄞", 4));
	ASSERT_EQ(" 𝄞𝄞𝄞", pad_middle_triple);
}*/
}
#endif

