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

#pragma once

#include "format_it.hpp"
#include <cstdint>
#include <type_traits>

namespace fmt
{
struct boolalpha_formatter
{
	bool value;
};
inline boolalpha_formatter boolalpha(bool value)
{
	return { value };
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, boolalpha_formatter value)
{
	if (value.value) return it.print("true");
	else return it.print("false");
}
template<typename T>
struct hex_formatter
{
	T value;
};
template<typename T>
hex_formatter<T> hex(T value)
{
	return { value };
}
template<typename T>
struct upperhex_formatter
{
	T value;
};
template<typename T>
upperhex_formatter<T> upperhex(T value)
{
	return { value };
}
template<typename T>
struct oct_formatter
{
	T value;
};
template<typename T>
oct_formatter<T> oct(T value)
{
	return { value };
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, bool value)
{
	if (value) return it.print('1');
	else return it.print('0');
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, char value)
{
	return it.print(C(value));
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, signed char value)
{
	return it.print(C(value));
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, unsigned char value)
{
	return it.print(C(value));
}
namespace detail
{
// integer printing method from Andrei Alexandrescu:
// https://www.facebook.com/notes/facebook-engineering/three-optimization-tips-for-c/10151361643253920
inline int digits8(uint16_t value)
{
	if (value < 010) return 1;
	else if (value < 0100) return 2;
	else if (value < 01000) return 3;
	else if (value < 010000) return 4;
	else if (value < 0100000) return 5;
	else return 6;
}
inline int digits8(uint32_t value)
{
	if (value < 010) return 1;
	else if (value < 0100) return 2;
	else if (value < 01000) return 3;
	else if (value < 010000) return 4;
	else if (value < 0100000) return 5;
	else if (value < 01000000000)
	{
		if (value < 010000000)
		{
			if (value < 01000000) return 6;
			else return 7;
		}
		else if (value < 0100000000) return 8;
		else return 9;
	}
	else if (value < 010000000000) return 10;
	else return 11;
}
inline int digits8(unsigned long long value)
{
	if (value < 010) return 1;
	else if (value < 0100) return 2;
	else if (value < 01000) return 3;
	else if (value < 010000) return 4;
	else if (value < 0100000000)
	{
		if (value < 01000000)
		{
			if (value < 0100000) return 5;
			else return 6;
		}
		else if (value < 010000000) return 7;
		else return 8;
	}
	else if (value < 010000000000llu)
	{
		if (value < 01000000000) return 9;
		else return 10;
	}
	else if (value < 0100000000000llu) return 11;
	else return 11 + digits8(value / 0100000000000llu);
}
inline int digits10(uint16_t value)
{
	if (value < 10) return 1;
	else if (value < 100) return 2;
	else if (value < 1000) return 3;
	else if (value < 10000) return 4;
	else return 5;
}
inline int digits10(uint32_t value)
{
	if (value < 10) return 1;
	else if (value < 100) return 2;
	else if (value < 1000) return 3;
	else if (value < 10000) return 4;
	else if (value < 100000) return 5;
	else if (value < 10000000)
	{
		if (value < 1000000) return 6;
		else return 7;
	}
	else if (value < 1000000000)
	{
		if (value < 100000000) return 8;
		else return 9;
	}
	else return 10;
}
inline int digits10(unsigned long long value)
{
	if (value < 10) return 1;
	else if (value < 100) return 2;
	else if (value < 1000) return 3;
	else if (value < 10000) return 4;
	else if (value < 100000000)
	{
		if (value < 1000000)
		{
			if (value < 100000) return 5;
			else return 6;
		}
		else if (value < 10000000) return 7;
		else return 8;
	}
	else if (value < 10000000000llu)
	{
		if (value < 1000000000) return 9;
		else return 10;
	}
	else return 10 + digits10(value / 10000000000llu);
}
inline int digits10(unsigned long value)
{
	return digits10(static_cast<unsigned long long>(value));
}
inline int digits16(uint16_t value)
{
	if (value < 0x10) return 1;
	else if (value < 0x100) return 2;
	else if (value < 0x1000) return 3;
	else return 4;
}
inline int digits16(uint32_t value)
{
	if (value < 0x10) return 1;
	else if (value < 0x100) return 2;
	else if (value < 0x1000) return 3;
	else if (value < 0x100000)
	{
		if (value < 0x10000) return 4;
		else return 5;
	}
	else if (value < 0x10000000)
	{
		if (value < 0x1000000) return 6;
		else return 7;
	}
	else return 8;
}
inline int digits16(unsigned long long value)
{
	if (value < 0x10) return 1;
	else if (value < 0x100) return 2;
	else if (value < 0x1000) return 3;
	else if (value < 0x10000000)
	{
		if (value < 0x100000)
		{
			if (value < 0x10000) return 4;
			else return 5;
		}
		else if (value < 0x1000000) return 6;
		else return 7;
	}
	else if (value < 0x100000000llu) return 8;
	else return 8 + digits16(value / 0x100000000llu);
}
inline int digits16(unsigned long value)
{
	return digits16(static_cast<unsigned long long>(value));
}
static constexpr const char oct_digits[129] =
		"0001020304050607"
		"1011121314151617"
		"2021222324252627"
		"3031323334353637"
		"4041424344454647"
		"5051525354555657"
		"6061626364656667"
		"7071727374757677";
template<typename T>
inline char * itoa_base8(T value, char * buffer)
{
	char * last = buffer + digits8(value);
	char * next = last - 1;
	while (value >= 0100)
	{
		auto index = (value % 0100) * 2;
		value /= 0100;
		*next-- = oct_digits[index + 1];
		*next-- = oct_digits[index];
	}
	if (value < 010) *next-- = '0' + value;
	else
	{
		auto index = value * 2;
		*next-- = oct_digits[index + 1];
		*next-- = oct_digits[index];
	}
	return last;
}
template<typename T>
inline char * itoa_base10(T value, char * buffer)
{
	static constexpr const char digits[201] =
			"00010203040506070809"
			"10111213141516171819"
			"20212223242526272829"
			"30313233343536373839"
			"40414243444546474849"
			"50515253545556575859"
			"60616263646566676869"
			"70717273747576777879"
			"80818283848586878889"
			"90919293949596979899";
	char * last = buffer + digits10(value);
	char * next = last - 1;
	while (value >= 100)
	{
		auto index = (value % 100) * 2;
		value /= 100;
		*next-- = digits[index + 1];
		*next-- = digits[index];
	}
	if (value < 10) *next-- = '0' + value;
	else
	{
		auto index = value * 2;
		*next-- = digits[index + 1];
		*next-- = digits[index];
	}
	return last;
}
static constexpr const char hex_lower_digits[513] =
		"000102030405060708090a0b0c0d0e0f"
		"101112131415161718191a1b1c1d1e1f"
		"202122232425262728292a2b2c2d2e2f"
		"303132333435363738393a3b3c3d3e3f"
		"404142434445464748494a4b4c4d4e4f"
		"505152535455565758595a5b5c5d5e5f"
		"606162636465666768696a6b6c6d6e6f"
		"707172737475767778797a7b7c7d7e7f"
		"808182838485868788898a8b8c8d8e8f"
		"909192939495969798999a9b9c9d9e9f"
		"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
		"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
		"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
		"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
		"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
		"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
static constexpr const char hex_upper_digits[513] =
		"000102030405060708090A0B0C0D0E0F"
		"101112131415161718191A1B1C1D1E1F"
		"202122232425262728292A2B2C2D2E2F"
		"303132333435363738393A3B3C3D3E3F"
		"404142434445464748494A4B4C4D4E4F"
		"505152535455565758595A5B5C5D5E5F"
		"606162636465666768696A6B6C6D6E6F"
		"707172737475767778797A7B7C7D7E7F"
		"808182838485868788898A8B8C8D8E8F"
		"909192939495969798999A9B9C9D9E9F"
		"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
template<char A, typename T>
inline char * itoa_base16(T value, char * buffer, const char digits[513])
{
	char * last = buffer + digits16(value);
	char * next = last - 1;
	while (value >= 0x100)
	{
		auto index = (value % 0x100) * 2;
		value /= 0x100;
		*next-- = digits[index + 1];
		*next-- = digits[index];
	}
	if (value < 10) *next-- = '0' + value;
	else if (value < 0x10) *next-- = A + value - 10;
	else
	{
		auto index = value * 2;
		*next-- = digits[index + 1];
		*next-- = digits[index];
	}
	return last;
}
}
template<typename C, typename It, typename T>
format_it<C, It> format_signed(format_it<C, It> it, T value)
{
	if (value < 0)
	{
		*it++ = C('-');
		return format(it, typename std::make_unsigned<T>::type(-value));
	}
	else return format(it, typename std::make_unsigned<T>::type(value));
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, uint16_t value)
{
	char buffer[5];
	return std::copy(buffer, detail::itoa_base10(value, buffer), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, int16_t value)
{
	return format_signed(it, value);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, uint32_t value)
{
	char buffer[10];
	return std::copy(buffer, detail::itoa_base10(value, buffer), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, int32_t value)
{
	return format_signed(it, value);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, unsigned long long value)
{
	char buffer[20];
	return std::copy(buffer, detail::itoa_base10(value, buffer), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, long long value)
{
	return format_signed(it, value);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, unsigned long value)
{
	return format(it, static_cast<unsigned long long>(value));
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, long value)
{
	return format_signed(it, value);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, hex_formatter<uint8_t> value)
{
	if (value.value < 10) return it.print(char('0' + value.value));
	else if (value.value < 0x10) return it.print(char('a' + value.value - 10));
	else
	{
		int index = int(value.value) * 2;
		*it++ = detail::hex_lower_digits[index];
		*it++ = detail::hex_lower_digits[index + 1];
		return it;
	}
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, hex_formatter<uint16_t> value)
{
	char buffer[4];
	return std::copy(buffer, detail::itoa_base16<'a'>(value.value, buffer, detail::hex_lower_digits), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, hex_formatter<uint32_t> value)
{
	char buffer[8];
	return std::copy(buffer, detail::itoa_base16<'a'>(value.value, buffer, detail::hex_lower_digits), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, hex_formatter<unsigned long long> value)
{
	char buffer[16];
	return std::copy(buffer, detail::itoa_base16<'a'>(value.value, buffer, detail::hex_lower_digits), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, hex_formatter<unsigned long> value)
{
	return format(it, hex(static_cast<unsigned long long>(value.value)));
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, hex_formatter<T> value)
{
	if (value.value < 0)
	{
		*it++ = C('-');
		return format(it, hex(typename std::make_unsigned<T>::type(-value.value)));
	}
	else return format(it, hex(typename std::make_unsigned<T>::type(value.value)));
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, upperhex_formatter<uint8_t> value)
{
	if (value.value < 10) return it.print(char('0' + value.value));
	else if (value.value < 0x10) return it.print(char('A' + value.value - 10));
	else
	{
		int index = int(value.value) * 2;
		*it++ = detail::hex_upper_digits[index];
		*it++ = detail::hex_upper_digits[index + 1];
		return it;
	}
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, upperhex_formatter<uint16_t> value)
{
	char buffer[4];
	return std::copy(buffer, detail::itoa_base16<'A'>(value.value, buffer, detail::hex_upper_digits), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, upperhex_formatter<uint32_t> value)
{
	char buffer[8];
	return std::copy(buffer, detail::itoa_base16<'A'>(value.value, buffer, detail::hex_upper_digits), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, upperhex_formatter<unsigned long long> value)
{
	char buffer[16];
	return std::copy(buffer, detail::itoa_base16<'A'>(value.value, buffer, detail::hex_upper_digits), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, upperhex_formatter<unsigned long> value)
{
	return format(it, upperhex(static_cast<unsigned long long>(value.value)));
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, upperhex_formatter<T> value)
{
	if (value.value < 0)
	{
		*it++ = C('-');
		return format(it, upperhex(typename std::make_unsigned<T>::type(-value.value)));
	}
	else return format(it, upperhex(typename std::make_unsigned<T>::type(value.value)));
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, oct_formatter<uint8_t> value)
{
	if (value.value < 010) return it.print(char('0' + value.value));
	else if (value.value >= 0100)
	{
		*it++ = char('0' + value.value / 0100);
		value.value %= 0100;
	}
	auto index = value.value * 2;
	*it++ = detail::oct_digits[index];
	*it++ = detail::oct_digits[index + 1];
	return it;
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, oct_formatter<uint16_t> value)
{
	char buffer[6];
	return std::copy(buffer, detail::itoa_base8(value.value, buffer), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, oct_formatter<uint32_t> value)
{
	char buffer[11];
	return std::copy(buffer, detail::itoa_base8(value.value, buffer), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, oct_formatter<unsigned long long> value)
{
	char buffer[22];
	return std::copy(buffer, detail::itoa_base8(value.value, buffer), it);
}
template<typename C, typename It>
format_it<C, It> format(format_it<C, It> it, oct_formatter<unsigned long> value)
{
	return format(it, oct(static_cast<unsigned long long>(value.value)));
}
template<typename C, typename It, typename T>
format_it<C, It> format(format_it<C, It> it, oct_formatter<T> value)
{
	if (value.value < 0)
	{
		*it++ = C('-');
		return format(it, oct(typename std::make_unsigned<T>::type(-value.value)));
	}
	else return format(it, oct(typename std::make_unsigned<T>::type(value.value)));
}
}
