#pragma once

#include <string>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <vector>
#include <deque>
#include <cstring>

#define FORMAT_NO_INLINE __attribute__((noinline))

namespace fmt
{
template<typename C, typename It>
struct format_it;
template<typename T, typename C, typename It, typename Enable = void>
struct formatter;
}

template<typename C, typename It, typename T>
fmt::format_it<C, It> format(fmt::format_it<C, It> it, const T & value)
{
	return fmt::formatter<T, C, It>()(it, value);
}

namespace fmt
{
namespace detail
{
template<typename C, typename It>
struct iterator_streambuf : std::basic_streambuf<C>
{
	typedef typename std::basic_streambuf<C>::int_type int_type;
	typedef typename std::basic_streambuf<C>::traits_type traits_type;
	iterator_streambuf(It it)
		: it(it)
	{
	}

	It it;

	virtual std::streamsize xsputn(const C * s, std::streamsize size) override
	{
		it = std::copy(s, s + size, it);
		return size;
	}
	virtual int_type overflow(int_type ch = traits_type::eof()) override
	{
		if (!traits_type::eq_int_type(ch, traits_type::eof())) *it++ = ch;
		return traits_type::eof() + 1;
	}
};
template<typename It, typename C, typename T>
format_it<C, It> adl_format(format_it<C, It> it, const T & value)
{
	return format(it, value);
}
}
struct format_error : std::runtime_error
{
	enum Reason
	{
		UnusedArgument,
		PercentNotFollowedByNumber,
		OpenPercentAtEndOfInput,
		FormatIndexOutOfRange
	};

	format_error(Reason reason);

	Reason reason() const
	{
		return _reason;
	}

private:
	Reason _reason;
};

template<typename C, typename It>
struct format_it : std::iterator<std::output_iterator_tag, void, void, void, void>
{
	format_it(It it)
		: _it(std::move(it))
	{
	}
	format_it & operator++()
	{
		return *this;
	}
	format_it & operator++(int)
	{
		return *this;
	}
	format_it & operator*()
	{
		return *this;
	}
	format_it & operator=(C c)
	{
		*_it++ = c;
		return *this;
	}
	template<typename T>
	format_it & operator=(const T & value)
	{
		return *this = detail::adl_format(*this, value);
	}

	template<size_t FormatSize, typename... Args>
	format_it & format(const C (&format_string)[FormatSize], const Args &... args)
	{
		return format(format_string + 0, format_string + FormatSize - 1, args...);
	}
	template<size_t FormatSize, typename First, typename... Args>
	format_it & format(const C (&format_string)[FormatSize], const First & first, const Args &... args)
	{
		return format(format_string + 0, format_string + FormatSize - 1, first, args...);
	}
	template<typename Traits, typename Allocator, typename... Args>
	format_it & format(const std::basic_string<C, Traits, Allocator> & format_string, const Args &... args)
	{
		return format(format_string.begin(), format_string.end(), args...);
	}
	template<typename Traits, typename Allocator, typename First, typename... Args>
	format_it & format(const std::basic_string<C, Traits, Allocator> & format_string, const First & first, const Args &... args)
	{
		return format(format_string.begin(), format_string.end(), first, args...);
	}
	template<typename BeginIt, typename EndIt, typename... Args>
	format_it & format(BeginIt begin, EndIt end, const Args &... args)
	{
		DefaultInitializedBool did_use_argument[sizeof...(Args)];
		while (begin != end)
		{
			if (*begin == C('%'))
			{
				++begin;
				if (begin == end)
				{
					throw format_error(format_error::OpenPercentAtEndOfInput);
				}
				else if (is_digit(*begin))
				{
					int i = to_digit(*begin);
					for (++begin; begin != end && is_digit(*begin); ++begin)
					{
						i *= 10;
						i += to_digit(*begin);
					}
					index_format(i, args...);
					did_use_argument[i].b = true;
				}
				else if (*begin == C('%'))
				{
					*this = C('%');
					++begin;
				}
				else
				{
					throw format_error(format_error::PercentNotFollowedByNumber);
				}
			}
			else
			{
				*this = *begin;
				++begin;
			}
		}
		if (!std::all_of(did_use_argument, did_use_argument + sizeof...(Args), [](DefaultInitializedBool b){ return b.b; }))
		{
			throw format_error(format_error::UnusedArgument);
		}
		return *this;
	}

	template<C Separator = ' ', typename First, typename Second, typename... Args>
	format_it print(const First & first, const Second & second, const Args &... args)
	{
		*this = first;
		*this = Separator;
		return print<Separator>(second, args...);
	}
	template<C Separator = ' ', typename First>
	format_it print(const First & first)
	{
		return *this = first;
	}
	template<typename First, typename... Args>
	format_it printpacked(const First & first, const Args &... args)
	{
		*this = first;
		return printpacked(args...);
	}
	template<typename First>
	format_it printpacked(const First & first)
	{
		return *this = first;
	}
	template<typename Separator, typename First, typename Second, typename... Args>
	format_it print_separated(const Separator & separator, const First & first, const Second & second, const Args &... args)
	{
		*this = first;
		*this = separator;
		return print_separated(separator, second, args...);
	}
	template<typename Separator, typename First>
	format_it print_separated(const Separator &, const First & first)
	{
		return *this = first;
	}

	It it() const
	{
		return _it;
	}
	void it(It it)
	{
		_it = std::move(it);
	}

private:
	It _it;

	void index_format(int)
	{
		throw format_error(format_error::FormatIndexOutOfRange);
	}
	template<typename First, typename... Args>
	void index_format(int i, const First & first, const Args &... args)
	{
		if (i == 0) *this = first;
		else index_format(i - 1, args...);
	}
	static bool is_digit(C c)
	{
		return c >= C('0') && c <= C('9');
	}
	static int to_digit(C c)
	{
		return c - C('0');
	}
	struct DefaultInitializedBool
	{
		bool b = false;
	};
};
template<typename C = char, typename It>
format_it<C, It> make_format_it(It it)
{
	return { std::move(it) };
}

template<typename T, typename C, typename It, typename Enable>
struct formatter
{
	format_it<C, It> operator()(format_it<C, It> it, const T & value) const
	{
		static thread_local detail::iterator_streambuf<C, format_it<C, It> > buf(it);
		static thread_local std::basic_ostream<C> stream(&buf);
		buf.it = it;
		stream << value;
		return buf.it;
	}
};
template<typename T, typename A, typename C, typename It>
format_it<C, It> format(format_it<C, It> it, const std::basic_string<C, T, A> & string)
{
	return std::copy(string.begin(), string.end(), it);
}
template<typename C, typename It, size_t Size>
format_it<C, It> format(format_it<C, It> it, const C (&string)[Size])
{
	return std::copy(string, string + Size - 1, it);
}
template<typename C, typename It>
struct formatter<const C *, C, It>
{
	format_it<C, It> operator()(format_it<C, It> it, const C * string) const
	{
		while (*string)
		{
			*it++ = *string++;
		}
		return it;
	}
};
template<typename C, typename It>
struct formatter<const char *, C, It>
{
	format_it<C, It> operator()(format_it<C, It> it, const char * string) const
	{
		while (*string)
		{
			*it++ = *string++;
		}
		return it;
	}
};
template<typename It>
struct formatter<const char *, char, It>
{
	format_it<char, It> operator()(format_it<char, It> it, const char * string) const
	{
		while (*string)
		{
			*it++ = *string++;
		}
		return it;
	}
};
template<typename C, size_t Size, typename It>
struct formatter<C[Size], C, It>
{
	format_it<C, It> operator()(format_it<C, It> it, const C (&string)[Size]) const
	{
		return std::copy(string, string + Size - 1, it);
	}
};
template<size_t Size, typename C, typename It>
struct formatter<char[Size], C, It>
{
	format_it<C, It> operator()(format_it<C, It> it, const char (&string)[Size]) const
	{
		return std::copy(string, string + Size - 1, it);
	}
};
template<size_t Size, typename It>
struct formatter<char[Size], char, It>
{
	format_it<char, It> operator()(format_it<char, It> it, const char (&string)[Size]) const
	{
		return std::copy(string, string + Size - 1, it);
	}
};
}

#include "format_integers.hpp"
#include "format_float.hpp"

