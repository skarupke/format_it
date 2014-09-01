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

#include "format_integers.hpp"

#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
#include "stack_format.hpp"
namespace
{
TEST(format_int, boolalpha)
{
	fmt::stack_print<1024> boolalpha(true, false, fmt::boolalpha(true), fmt::boolalpha(false));
	ASSERT_EQ(std::string("1 0 true false"), boolalpha.c_str());
}
TEST(format_int, characters)
{
	fmt::stack_print<1024> foo(char(65), static_cast<signed char>(65), static_cast<unsigned char>(65));
	ASSERT_EQ(std::string("A A A"), foo.c_str());
}
TEST(format_int, integers)
{
	fmt::stack_print<1024> positive(short(12345), 1234567890, 12345678901234567890llu);
	ASSERT_EQ(std::string("12345 1234567890 12345678901234567890"), positive.c_str());
	fmt::stack_print<1024> limit(static_cast<unsigned short>(65535), 4294967295u, 18446744073709551615llu);
	ASSERT_EQ(std::string("65535 4294967295 18446744073709551615"), limit.c_str());
	fmt::stack_print<1024> negative(short(-32768), -2147483647 - 1, -9223372036854775807ll - 1);
	ASSERT_EQ(std::string("-32768 -2147483648 -9223372036854775808"), negative.c_str());
	fmt::stack_print<1024> digits_count_bug(-2147483648); // this would give different results than the above test because the type is no longer int
	ASSERT_EQ(std::string("-2147483648"), digits_count_bug.c_str());
	fmt::stack_print<1024> all(-1, 22, -333, 4444, -55555, 666666, -7777777, 88888888, -999999999, 1010101010, -11111111111ll, 121212121212llu, -1313131313131ll, 14141414141414llu, -151515151515151ll, 1616161616161616llu, -17171717171717171ll, 181818181818181818llu, -1919191919191919191ll, 12020202020202020202llu);
	ASSERT_EQ(std::string("-1 22 -333 4444 -55555 666666 -7777777 88888888 -999999999 1010101010 -11111111111 121212121212 -1313131313131 14141414141414 -151515151515151 1616161616161616 -17171717171717171 181818181818181818 -1919191919191919191 12020202020202020202"), all.c_str());
}
TEST(format_int, hex)
{
	fmt::stack_print<1024> hex(fmt::hex(static_cast<unsigned char>(0xfe)), fmt::hex(0x1234567890abcdefllu));
	ASSERT_EQ(std::string("fe 1234567890abcdef"), hex.c_str());
	fmt::stack_print<1024> upperhex(fmt::upperhex(static_cast<unsigned char>(0xfe)), fmt::upperhex(0x1234567890abcdefllu));
	ASSERT_EQ(std::string("FE 1234567890ABCDEF"), upperhex.c_str());
	fmt::stack_print<1024> hex_single_char(fmt::hex(char(5)), fmt::hex(char(0xa)));
	ASSERT_EQ(std::string("5 a"), hex_single_char.c_str());
	fmt::stack_print<1024> upperhex_single_char(fmt::upperhex(char(5)), fmt::upperhex(char(0xa)), fmt::upperhex(0xb));
	ASSERT_EQ(std::string("5 A B"), upperhex_single_char.c_str());
}
TEST(format_int, oct)
{
	fmt::stack_print<1024> oct(fmt::oct(static_cast<unsigned char>(0255)), fmt::oct(012345670));
	ASSERT_EQ(std::string("255 12345670"), oct.c_str());
	fmt::stack_print<1024> single_oct(fmt::oct(static_cast<char>(5)), fmt::oct(5));
	ASSERT_EQ(std::string("5 5"), single_oct.c_str());
}
}
#endif
