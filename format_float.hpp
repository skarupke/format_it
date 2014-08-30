#pragma once

#include "format_it.hpp"
#include "format_integers.hpp"
#include "double_conversion/fast-dtoa.h"
#include "double_conversion/double-conversion.h"
#include "double_conversion/bignum-dtoa.h"
#include <iterator>

namespace fmt
{
namespace detail
{
struct FloatSplit
{
	FloatSplit(double as_double)
		: as_double(as_double)
	{
	}

	union
	{
		double as_double;
		struct
		{
			uint64_t mantissa : 52;
			unsigned short exponent : 11;
			bool negative : 1;
		};
	};
};
static_assert(sizeof(FloatSplit) == sizeof(double), "I use that struct to look at doubles");

template<typename C, typename It, typename T>
struct DtoaStart
{
	format_it<C, It> it;
	bool finished;
	T new_value;
};
template<typename C, typename It, typename T>
DtoaStart<C, It, T> start_dtoa(format_it<C, It> it, T value)
{
	if (FloatSplit(value).negative)
	{
		*it++ = '-';
		value = -value;
	}
	if (std::isnan(value)) return { it.print("nan"), true, value };
	else if (value == 0.0) return { it.print('0'), true, value };
	else if (value == std::numeric_limits<double>::infinity()) return { it.print("inf"), true, value };
	else return { it, false, value };
}

template<size_t Size>
double_conversion::FastDtoaResult dtoa_into_buffer(char (&buffer)[Size], double value, double_conversion::FastDtoaMode dtoa_mode, double_conversion::BignumDtoaMode fallback_mode, int num_digits)
{
	double_conversion::FastDtoaResult result = double_conversion::FastDtoa(value, dtoa_mode, num_digits, double_conversion::Vector<char>(buffer, Size));
	if (!result.succeeded)
	{
		double_conversion::BignumDtoa(value, fallback_mode, num_digits, double_conversion::Vector<char>(buffer, Size), &result.length, &result.decimal_point);
	}
	return result;
}

// this function matches libstdc++ printf("%g", value) for all single precision
// floating point numbers. I didn't run tests for double precision but the
// results are probably not too far off
template<typename C, typename It>
format_it<C, It> printf_style_dtoa(format_it<C, It> it, double value, int num_digits = 6)
{
	auto start = start_dtoa(it, value);
	if (start.finished) return start.it;
	it = start.it;
	value = start.new_value;

	char buffer[1024];
	double_conversion::FastDtoaResult result = dtoa_into_buffer(buffer, value, double_conversion::FAST_DTOA_PRECISION, double_conversion::BIGNUM_DTOA_PRECISION, num_digits);
	char * last_non_zero = std::find_if(std::reverse_iterator<char *>(buffer + result.length), std::reverse_iterator<char *>(buffer), [](char c){ return c != '0'; }).base();
	auto start_scientific = [&]
	{
		*it++ = buffer[0];
		--result.decimal_point;
		if (last_non_zero != buffer + 1)
		{
			*it++ = '.';
			it = std::copy(buffer + 1, last_non_zero, it);
		}
		*it++ = 'e';
	};
	if (result.decimal_point < -3)
	{
		start_scientific();
		if (result.decimal_point > -10) return it.printpacked("-0", -result.decimal_point);
		else return it.print(result.decimal_point);
	}
	else if (result.decimal_point > num_digits)
	{
		start_scientific();
		if (result.decimal_point < 10) return it.printpacked("+0", result.decimal_point);
		else return it.printpacked('+', result.decimal_point);
	}
	else if (result.decimal_point < 1)
	{
		*it++ = "0.";
		it = std::fill_n(it, 0 - result.decimal_point, '0');
		return std::copy(buffer, last_non_zero, it);
	}
	else
	{
		char * decimal = buffer + result.decimal_point;
		char * mid = std::min(decimal, last_non_zero);
		it = std::copy(buffer, mid, it);
		if (mid != last_non_zero)
		{
			*it++ = '.';
			return it = std::copy(mid, last_non_zero, it);
		}
		else if (mid != decimal) return std::copy(mid, decimal, it);
		else return it;
	}
}
template<typename C, typename It>
format_it<C, It> finish_fixed_dtoa(format_it<C, It> it, char * buffer, double_conversion::FastDtoaResult result)
{
	char * end = buffer + result.length;
	if (result.decimal_point < 1)
	{
		*it++ = "0.";
		it = std::fill_n(it, 0 - result.decimal_point, '0');
		return std::copy(buffer, end, it);
	}
	else
	{
		char * decimal = buffer + result.decimal_point;
		if (decimal < end)
		{
			it = std::copy(buffer, decimal, it);
			*it++ = '.';
			return it = std::copy(decimal, end, it);
		}
		else
		{
			it = std::copy(buffer, end, it);
			return std::fill_n(it, decimal - end, '0');
		}
	}
}
template<typename C, typename It>
format_it<C, It> fixed_dtoa(format_it<C, It> it, double value)
{
	auto start = start_dtoa(it, value);
	if (start.finished) return start.it;

	char buffer[1024];
	return finish_fixed_dtoa(start.it, buffer, dtoa_into_buffer(buffer, start.new_value, double_conversion::FAST_DTOA_SHORTEST, double_conversion::BIGNUM_DTOA_SHORTEST, 0));
}
template<typename C, typename It>
format_it<C, It> fixed_dtoa(format_it<C, It> it, float value)
{
	auto start = start_dtoa(it, value);
	if (start.finished) return start.it;

	char buffer[1024];
	return finish_fixed_dtoa(start.it, buffer, dtoa_into_buffer(buffer, start.new_value, double_conversion::FAST_DTOA_SHORTEST_SINGLE, double_conversion::BIGNUM_DTOA_SHORTEST_SINGLE, 0));
}
inline char round_char_to_even(char c, char next, bool has_more)
{
	if ((c - '0') % 2)
	{
		if (next >= '5') return c + 1;
		else return c;
	}
	else if (next == '5')
	{
		if (has_more) return c + 1;
		else return c;
	}
	else if (next > '5') return c + 1;
	else return c;
}
inline char * round_digits(char * begin, char * past_end, bool has_more)
{
	auto it = std::reverse_iterator<char *>(past_end);
	auto end = std::reverse_iterator<char *>(begin);
	char last = *it;
	for (++it; it != end; ++it)
	{
		*it = last = round_char_to_even(*it, last, has_more);
		if (last != '0' + 10) return it.base();
		has_more = true;
	}
	*begin = '1';
	return begin;
}
inline bool round_buffer(char * begin, char * print_end, char * end)
{
	char * rounded_end = round_digits(begin, print_end + 1, print_end != end - 1);
	bool overflowed = rounded_end == begin;
	if (overflowed)
	{
		++rounded_end;
	}
	std::fill(rounded_end, print_end, '0');
	return overflowed;
}
template<typename C, typename It>
format_it<C, It> fixed_width_dtoa(format_it<C, It> it, double value, int num_digits)
{
	if (FloatSplit(value).negative)
	{
		*it++ = '-';
		value = -value;
		--num_digits;
	}
	if (std::isnan(value)) return std::fill_n(it.print("nan"), std::max(0, num_digits - 3), ' ');
	else if (value == 0.0) return std::fill_n(it.print("0."), num_digits - 2, '0');
	else if (value == std::numeric_limits<double>::infinity()) return std::fill_n(it.print("inf"), std::max(0, num_digits - 3), ' ');

	char buffer[1024];
	double_conversion::FastDtoaResult result = dtoa_into_buffer(buffer, value, double_conversion::FAST_DTOA_PRECISION, double_conversion::BIGNUM_DTOA_PRECISION, num_digits);
	auto start_scientific = [&]
	{
		--result.decimal_point;
		int exponent_length = std::abs(result.decimal_point) < 100 ? 2 : 3;
		char * print_end = buffer + num_digits - exponent_length - 3;
		if (print_end > buffer + 1)
		{
			char * end = buffer + result.length;
			if (print_end < end && round_buffer(buffer, print_end, end))
			{
				++result.decimal_point;
			}
			*it++ = buffer[0];
			*it++ = '.';
			it = std::copy(buffer + 1, print_end, it);
		}
		else *it++ = buffer[0];
		*it++ = 'e';
	};
	if (result.decimal_point < std::max(-3, -num_digits + 3))
	{
		start_scientific();
		if (result.decimal_point > -10) return it.printpacked("-0", -result.decimal_point);
		else return it.print(result.decimal_point);
	}
	else if (result.decimal_point > num_digits)
	{
		start_scientific();
		if (result.decimal_point < 10) return it.printpacked("+0", result.decimal_point);
		else return it.printpacked('+', result.decimal_point);
	}
	else if (result.decimal_point < 1)
	{
		char * print_end = buffer + num_digits - 2 + result.decimal_point;
		char * end = buffer + result.length;
		if (round_buffer(buffer, print_end, end) && result.decimal_point == 0)
		{
			*it++ = "1.";
			return std::fill_n(it, num_digits - 2, '0');
		}
		else
		{
			*it++ = "0.";
			it = std::fill_n(it, 0 - result.decimal_point, '0');
			return std::copy(buffer, print_end, it);
		}
	}
	else if (result.decimal_point < num_digits)
	{
		char * print_end = buffer + num_digits - 1;
		round_buffer(buffer, print_end, print_end + 1);
		char * decimal = buffer + result.decimal_point;
		it = std::copy(buffer, decimal, it);
		*it++ = '.';
		return it = std::copy(decimal, print_end, it);
	}
	else return std::copy(buffer, buffer + num_digits, it);
}
template<typename C, typename It>
format_it<C, It> ecma_style_dtoa(format_it<C, It> it, double value)
{
	char buffer[1024];
	double_conversion::StringBuilder builder(buffer, sizeof(buffer) / sizeof(*buffer));
	double_conversion::DoubleToStringConverter::EcmaScriptConverter().ToShortest(value, builder);
	return std::copy(buffer, buffer + builder.position(), it);
}
template<typename C, typename It>
format_it<C, It> ecma_style_dtoa(format_it<C, It> it, float value)
{
	char buffer[1024];
	double_conversion::StringBuilder builder(buffer, sizeof(buffer) / sizeof(*buffer));
	double_conversion::DoubleToStringConverter::EcmaScriptConverter().ToShortestSingle(value, builder);
	return std::copy(buffer, buffer + builder.position(), it);
}
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, float value)
{
	return detail::printf_style_dtoa(it, value);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, double value)
{
	return detail::printf_style_dtoa(it, value);
}
template<typename T>
struct nodrift_float_formatter
{
	T value;
};
template<typename T>
nodrift_float_formatter<T> nodrift_float(T value)
{
	return { value };
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, nodrift_float_formatter<T> value)
{
	return detail::ecma_style_dtoa(it, value.value);
}
template<typename T>
struct precise_float_formatter
{
	T value;
	int num_digits;
};
template<typename T>
precise_float_formatter<T> precise_float(T value, int num_digits)
{
	return { value, num_digits };
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, precise_float_formatter<T> value)
{
	return detail::printf_style_dtoa(it, value.value, value.num_digits);
}
template<typename T>
struct pad_float_formatter
{
	T value;
	int num_digits;
};
template<typename T>
pad_float_formatter<T> pad_float(T value, int num_digits)
{
	return { value, num_digits };
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, pad_float_formatter<T> value)
{
	return detail::fixed_width_dtoa(it, value.value, value.num_digits);
}
template<typename T>
struct fixed_float_formatter
{
	T value;
};
template<typename T>
fixed_float_formatter<T> float_as_fixed(T value)
{
	return { value };
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, fixed_float_formatter<T> value)
{
	return detail::fixed_dtoa(it, value.value);
}
}
