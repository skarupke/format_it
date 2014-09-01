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

#include "format_float.hpp"

#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
#include "stack_format.hpp"
#include <cstdio>

namespace
{
TEST(format_float, round_char_to_even)
{
	ASSERT_EQ('0', fmt::detail::round_char_to_even('0', '3', false));
	ASSERT_EQ('0', fmt::detail::round_char_to_even('0', '5', false));
	ASSERT_EQ('1', fmt::detail::round_char_to_even('0', '7', false));
	ASSERT_EQ('1', fmt::detail::round_char_to_even('1', '3', false));
	ASSERT_EQ('2', fmt::detail::round_char_to_even('1', '5', false));
	ASSERT_EQ('2', fmt::detail::round_char_to_even('1', '7', false));
	ASSERT_EQ('0', fmt::detail::round_char_to_even('0', '3', true));
	ASSERT_EQ('1', fmt::detail::round_char_to_even('0', '5', true));
	ASSERT_EQ('1', fmt::detail::round_char_to_even('0', '7', true));
	ASSERT_EQ('1', fmt::detail::round_char_to_even('1', '3', true));
	ASSERT_EQ('2', fmt::detail::round_char_to_even('1', '5', true));
	ASSERT_EQ('2', fmt::detail::round_char_to_even('1', '7', true));
}

::testing::AssertionResult TestFloatEqualToPrintf(double value)
{
	char buffer[32];
	int printed = snprintf(buffer, sizeof(buffer) / sizeof(*buffer), "%g", value);
	fmt::stack_print<1024> formatted(value);
	if (printed == int(formatted.size()) && std::equal(formatted.begin(), formatted.end(), buffer))
	{
		return ::testing::AssertionSuccess();
	}
	else return ::testing::AssertionFailure() << "Number " << value << " printed as " << buffer << " using snprintf and as " << formatted.c_str() << " using format_it. They should be equal";
}
inline bool double_exact_equal(double a, double b)
{
	if (std::isnan(a)) return std::isnan(b);
	else return a == b;
}
::testing::AssertionResult TestPrintAndReadBackLosslessly(double value)
{
	fmt::stack_print<1024> formatted(fmt::nodrift_float(value));
	double read = 0.0;
	if (sscanf(formatted.c_str(), "%lf", &read) != 1)
	{
		return ::testing::AssertionFailure() << "Number " << value << " printed as " << formatted.c_str() << " could not be read back.";
	}
	else if (double_exact_equal(value, read))
	{
		return ::testing::AssertionSuccess();
	}
	else return ::testing::AssertionFailure() << "Number " << value << " printed as " << formatted.c_str() << " but read back as " << fmt::stack_print<1024>(fmt::nodrift_float(read)).c_str() << ". They should be equal.";
}
::testing::AssertionResult TestPrecision(double value)
{
	fmt::stack_format<1024> formatted("%0, %1", fmt::precise_float(value, 3), fmt::precise_float(value, 9));
	char buffer[32];
	int printed = snprintf(buffer, sizeof(buffer) / sizeof(*buffer), "%.3g, %.9g", value, value);
	if (printed == int(formatted.size()) && std::equal(formatted.begin(), formatted.end(), buffer))
	{
		return ::testing::AssertionSuccess();
	}
	else return ::testing::AssertionFailure() << "Number " << value << " printed as " << buffer << " using snprintf and as " << formatted.c_str() << " using format_it. They should be equal";
}

::testing::AssertionResult TestDoublePrinting(double value)
{
	::testing::AssertionResult equal_to_printf = TestFloatEqualToPrintf(value);
	if (!equal_to_printf) return equal_to_printf;
	::testing::AssertionResult lossless = TestPrintAndReadBackLosslessly(value);
	if (!lossless) return lossless;
	return TestPrecision(value);
}

TEST(format_float, zeros)
{
	ASSERT_TRUE(TestDoublePrinting(0.0));
	ASSERT_TRUE(TestDoublePrinting(-0.0));
}
TEST(format_float, nan)
{
	ASSERT_TRUE(TestDoublePrinting(0.0 / 0.0));
}
TEST(format_float, infinities)
{
	ASSERT_TRUE(TestDoublePrinting(std::numeric_limits<double>::infinity()));
	ASSERT_TRUE(TestDoublePrinting(-std::numeric_limits<double>::infinity()));
}
TEST(format_float, near_origin)
{
	ASSERT_TRUE(TestDoublePrinting(0.1));
	ASSERT_TRUE(TestDoublePrinting(-0.1));
	ASSERT_TRUE(TestDoublePrinting(1.0));
	ASSERT_TRUE(TestDoublePrinting(-1.0));
	ASSERT_TRUE(TestDoublePrinting(2.0));
	ASSERT_TRUE(TestDoublePrinting(-2.0));
}
TEST(format_float, medium_numbers)
{
	ASSERT_TRUE(TestDoublePrinting(1000.0f));
}
TEST(format_float, large_numbers)
{
	ASSERT_TRUE(TestDoublePrinting(123456789123456789.0));
	ASSERT_TRUE(TestDoublePrinting(-123456789123456789.0));
	ASSERT_TRUE(TestDoublePrinting(34200000000000.0));
	ASSERT_TRUE(TestDoublePrinting(-3420000000000000.0));
}
TEST(format_float, specifics)
{
	// specific numbers that have failed in the past
	ASSERT_TRUE(TestDoublePrinting(3.7835058536770061e-44));
	ASSERT_TRUE(TestDoublePrinting(2.8025969286496341e-44));
	ASSERT_TRUE(TestDoublePrinting(1e-10));
	ASSERT_TRUE(TestDoublePrinting(2.0000032332075952e-40));
	ASSERT_TRUE(TestDoublePrinting(9.9999979491579396e-39));
	ASSERT_TRUE(TestDoublePrinting(1.0000003385357559e-05));
	ASSERT_TRUE(TestDoublePrinting(0.0047237873077392578));
	ASSERT_TRUE(TestDoublePrinting(0.005859375));
	ASSERT_TRUE(TestDoublePrinting(1.0000052452087402));
	ASSERT_TRUE(TestDoublePrinting(9.9999963865116115e10));
	ASSERT_TRUE(TestDoublePrinting(9.9999963865116115e8));
	ASSERT_TRUE(TestDoublePrinting(9.9999963865116115e-10));
	ASSERT_TRUE(TestDoublePrinting(-9.9950004369020462e-05));
}

TEST(format_float, limits)
{
	ASSERT_TRUE(TestDoublePrinting(std::numeric_limits<double>::max()));
	ASSERT_TRUE(TestDoublePrinting(std::numeric_limits<double>::min()));
	ASSERT_TRUE(TestDoublePrinting(-std::numeric_limits<double>::min()));
	ASSERT_TRUE(TestDoublePrinting(std::numeric_limits<double>::lowest()));
}

TEST(format_float, pad_float)
{
	ASSERT_EQ("-1.00", fmt::stack_print<1024>(fmt::pad_float(-1, 5)));
	ASSERT_EQ("0.001", fmt::stack_print<1024>(fmt::pad_float(0.001, 5)));
	ASSERT_EQ("1e-04", fmt::stack_print<1024>(fmt::pad_float(0.0001, 5)));
	ASSERT_EQ("-1e-04", fmt::stack_print<1024>(fmt::pad_float(-0.0001, 5)));
	ASSERT_EQ("0.0001", fmt::stack_print<1024>(fmt::pad_float(0.0001, 6)));
	ASSERT_EQ("-1e-04", fmt::stack_print<1024>(fmt::pad_float(-0.0001, 6)));
	ASSERT_EQ("1e-09", fmt::stack_print<1024>(fmt::pad_float(1e-9, 5)));
	ASSERT_EQ("100.0", fmt::stack_print<1024>(fmt::pad_float(100, 5)));
	ASSERT_EQ("1000.", fmt::stack_print<1024>(fmt::pad_float(1000, 5)));
	ASSERT_EQ("10000", fmt::stack_print<1024>(fmt::pad_float(10000, 5)));
	ASSERT_EQ("1e+06", fmt::stack_print<1024>(fmt::pad_float(1000000, 5)));
	ASSERT_EQ("3.7835e-44", fmt::stack_print<1024>(fmt::pad_float(3.7835058536770061e-44, 10)));
	ASSERT_EQ("2.8026e-44", fmt::stack_print<1024>(fmt::pad_float(2.8025969286496341e-44, 10)));
	ASSERT_EQ("1.1000e-11", fmt::stack_print<1024>(fmt::pad_float(1.09999e-11, 10)));
	ASSERT_EQ("2.0000e-11", fmt::stack_print<1024>(fmt::pad_float(1.99999e-11, 10)));
	ASSERT_EQ("1.0000e-10", fmt::stack_print<1024>(fmt::pad_float(1e-10, 10)));
	ASSERT_EQ("2.0000e-40", fmt::stack_print<1024>(fmt::pad_float(2.0000032332075952e-40, 10)));
	ASSERT_EQ("1.0000e-38", fmt::stack_print<1024>(fmt::pad_float(9.9999979491579396e-39, 10)));
	ASSERT_EQ("1.0000e-05", fmt::stack_print<1024>(fmt::pad_float(1.0000003385357559e-05, 10)));
	ASSERT_EQ("0.00472379", fmt::stack_print<1024>(fmt::pad_float(0.0047237873077392578, 10)));
	ASSERT_EQ("0.00585938", fmt::stack_print<1024>(fmt::pad_float(0.005859375, 10)));
	ASSERT_EQ("1.00000524", fmt::stack_print<1024>(fmt::pad_float(1.0000052452087402, 10)));
	ASSERT_EQ("1.0000e+11", fmt::stack_print<1024>(fmt::pad_float(9.9999963865116115e10, 10)));
	ASSERT_EQ("999999639.", fmt::stack_print<1024>(fmt::pad_float(9.9999963865116115e8, 10)));
	ASSERT_EQ("-999999639", fmt::stack_print<1024>(fmt::pad_float(-9.9999963865116115e8, 10)));
	ASSERT_EQ("9999996387", fmt::stack_print<1024>(fmt::pad_float(9.9999963865116115e9, 10)));
	ASSERT_EQ("1.0000e+10", fmt::stack_print<1024>(fmt::pad_float(9.9999999996e9, 10)));
	ASSERT_EQ("-1.000e+10", fmt::stack_print<1024>(fmt::pad_float(-9.9999963865116115e9, 10)));
	ASSERT_EQ("-9.995e-05", fmt::stack_print<1024>(fmt::pad_float(-9.9950004369020462e-05, 10)));
	ASSERT_EQ("1.00000000", fmt::stack_print<1024>(fmt::pad_float(0.999999996, 10)));
}
TEST(format_float, pad_float_special)
{
	ASSERT_EQ("0.00000000", fmt::stack_print<1024>(fmt::pad_float(0.0, 10)));
	ASSERT_EQ("-0.0000000", fmt::stack_print<1024>(fmt::pad_float(-0.0, 10)));
	//ASSERT_EQ("-nan      ", fmt::stack_print<1024>(fmt::pad_float(0.0 / 0.0, 10)));
	ASSERT_EQ("-inf      ", fmt::stack_print<1024>(fmt::pad_float(-std::numeric_limits<double>::infinity(), 10)));
	ASSERT_EQ("inf       ", fmt::stack_print<1024>(fmt::pad_float(std::numeric_limits<double>::infinity(), 10)));
}
TEST(format_float, fixed)
{
	ASSERT_EQ("1", fmt::stack_print<1024>(fmt::float_as_fixed(1.0)));
	ASSERT_EQ("-1", fmt::stack_print<1024>(fmt::float_as_fixed(-1.0)));
	ASSERT_EQ("0.001", fmt::stack_print<1024>(fmt::float_as_fixed(0.001)));
	ASSERT_EQ("0.0001", fmt::stack_print<1024>(fmt::float_as_fixed(0.0001)));
	ASSERT_EQ("-0.0001", fmt::stack_print<1024>(fmt::float_as_fixed(-0.0001)));
	ASSERT_EQ("0.000000001", fmt::stack_print<1024>(fmt::float_as_fixed(1e-9)));
	ASSERT_EQ("10000", fmt::stack_print<1024>(fmt::float_as_fixed(10000.0)));
	ASSERT_EQ("0.00000000000000000000000000000000000000000003783505853677006", fmt::stack_print<1024>(fmt::float_as_fixed(3.7835058536770061e-44)));
	ASSERT_EQ("0.0000000000109999", fmt::stack_print<1024>(fmt::float_as_fixed(1.09999e-11)));
	ASSERT_EQ("0.0000000001", fmt::stack_print<1024>(fmt::float_as_fixed(1e-10)));
	ASSERT_EQ("0.000010000003385357559", fmt::stack_print<1024>(fmt::float_as_fixed(1.0000003385357559e-05)));
	ASSERT_EQ("0.000010000003", fmt::stack_print<1024>(fmt::float_as_fixed(1.0000003385357559e-05f)));
	ASSERT_EQ("0.004723787307739258", fmt::stack_print<1024>(fmt::float_as_fixed(0.0047237873077392578)));
	ASSERT_EQ("1.0000052452087402", fmt::stack_print<1024>(fmt::float_as_fixed(1.0000052452087402)));
	ASSERT_EQ("99999963865.11612", fmt::stack_print<1024>(fmt::float_as_fixed(9.9999963865116115e10)));
	ASSERT_EQ("378350585367700600000000000000000000000000000", fmt::stack_print<1024>(fmt::float_as_fixed(3.7835058536770061e44)));
}

struct UnsignedAsFloat
{
	UnsignedAsFloat(unsigned value)
		: as_unsigned(value)
	{
	}

	union
	{
		float as_float;
		unsigned as_unsigned;
	};
};

constexpr unsigned num_iterations = 200000;
TEST(format_float, DISABLED_printf_performance)
{
	for (unsigned i = 0; i < num_iterations; ++i)
	{
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) / sizeof(*buffer), "%g, %g, %g, %g", static_cast<float>(i), std::pow(i, 1.45123), i * 0.5, UnsignedAsFloat(i).as_float);
	}
}
TEST(format_float, DISABLED_format_float_performance)
{
	for (unsigned i = 0; i < num_iterations; ++i)
	{
		fmt::stack_format<1024>("%0, %1, %2, %3", static_cast<float>(i), std::pow(i, 1.45123), i * 0.5, UnsignedAsFloat(i).as_float);
	}
}
}
#include <thread>
namespace
{
TEST(format_float, DISABLED_test_all_floats)
{
	std::vector<std::thread> threads;
	constexpr unsigned start = 0;
	constexpr int num_threads = 4;
	constexpr int step = num_threads;// * 1024;
	for (unsigned i = 0; i < num_threads; ++i)
	{
		threads.emplace_back([i]
		{
			for (unsigned j = i + start;;)
			{
				ASSERT_TRUE(TestDoublePrinting(UnsignedAsFloat(j).as_float));
				j += step;
				if (!(j % 0x100000))
				{
					std::cout << fmt::stack_format<1024>("0x%0, %1%%", fmt::hex(j), j / 42949672.96) << std::endl;
				}
				if (j == i) break;
			}
		});
	}
	for (std::thread & thread : threads)
	{
		thread.join();
	}
}
}
#endif

