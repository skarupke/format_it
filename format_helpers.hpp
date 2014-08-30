#pragma once

#include "stack_format.hpp"

namespace fmt
{
template<typename C, typename BeginIt, typename EndIt>
struct separated_formatter
{
	BeginIt begin;
	EndIt end;
	const C * separator;
};
template<typename BeginIt, typename EndIt, typename C>
separated_formatter<C, BeginIt, EndIt> with_separator(BeginIt begin, EndIt end, const C * separator)
{
	return { begin, end, separator };
}
template<typename T, typename C>
separated_formatter<C, typename T::const_iterator, typename T::const_iterator> with_separator(const T & range, const C * separator)
{
	return { range.begin(), range.end(), separator };
}
template<typename C, typename It, typename BeginIt, typename EndIt>
format_it<C, It> format(format_it<C, It> it, separated_formatter<C, BeginIt, EndIt> format)
{
	if (format.begin != format.end)
	{
		*it++ = *format.begin;
		for (++format.begin; format.begin != format.end; ++format.begin)
		{
			*it++ = format.separator;
			*it++ = *format.begin;
		}
	}
	return it;
}
namespace detail
{
template<typename C, typename It, typename T>
struct separator_it : std::iterator<std::output_iterator_tag, void, void, void, void>
{
	separator_it(format_it<C, It> it, T separator)
		: it(std::move(it)), separator(std::move(separator))
	{
	}

	separator_it & operator*()
	{
		return *this;
	}
	separator_it & operator++()
	{
		return *this;
	}
	separator_it & operator++(int)
	{
		return *this;
	}
	template<typename U>
	separator_it & operator=(const U & value)
	{
		if (first) first = false;
		else it.print(separator);
		it.print(value);
		return *this;
	}

	format_it<C, It> it;
	bool first = true;
	T separator;
};
}
template<typename C, typename It, typename T>
detail::separator_it<C, format_it<C, It>, T> with_separator(format_it<C, It> it, T separator)
{
	return detail::separator_it<C, format_it<C, It>, T>(std::move(it), std::move(separator));
}

namespace detail
{
template<typename V, typename C>
struct padded_formatter
{
	padded_formatter(const V & value, int num_padding, C padding)
		: value(value), num_padding(num_padding), padding(padding)
	{
	}
	const V & value;
	int num_padding;
	C padding;
};
}
template<typename V, typename C>
struct left_padded_formatter : detail::padded_formatter<V, C>
{
	using detail::padded_formatter<V, C>::padded_formatter;
};
template<typename V, typename C = char>
left_padded_formatter<V, C> pad_left(const V & value, int num_padding, C padding = C(' '))
{
	return { value, num_padding, padding };
}
template<typename C, typename It, typename V>
format_it<C, It> format(format_it<C, It> it, const left_padded_formatter<V, C> & padding)
{
	stack_print<1024> print_value(padding.value);
	it = std::fill_n(it, std::max(0, padding.num_padding - int(print_value.end() - print_value.begin())), padding.padding);
	return std::copy(print_value.begin(), print_value.end(), it);
}
template<typename V, typename C>
struct right_padded_formatter : detail::padded_formatter<V, C>
{
	using detail::padded_formatter<V, C>::padded_formatter;
};
template<typename V, typename C = char>
right_padded_formatter<V, C> pad_right(const V & value, int num_padding, C padding = C(' '))
{
	return { value, num_padding, padding };
}
template<typename C, typename It, typename V>
format_it<C, It> format(format_it<C, It> it, const right_padded_formatter<V, C> & padding)
{
	// the temporary storage isn't really needed for the right_padded_formatter
	// but it is needed for padding on the left, and I think they should be similar
	// in terms of both code and performance
	stack_print<1024> print_value(padding.value);
	it = std::copy(print_value.begin(), print_value.end(), it);
	return std::fill_n(it, std::max(0, padding.num_padding - int(print_value.end() - print_value.begin())), padding.padding);
}
template<typename V, typename C>
struct both_padded_formatter : detail::padded_formatter<V, C>
{
	using detail::padded_formatter<V, C>::padded_formatter;
};
template<typename V, typename C = char>
both_padded_formatter<V, C> pad_both(const V & value, int num_padding, C padding = C(' '))
{
	return { value, num_padding, padding };
}
template<typename C, typename It, typename V>
format_it<C, It> format(format_it<C, It> it, const both_padded_formatter<V, C> & padding)
{
	stack_print<1024> print_value(padding.value);
	int to_pad = std::max(0, padding.num_padding - int(print_value.end() - print_value.begin()));
	int right_half = to_pad / 2;
	it = std::fill_n(it, to_pad - right_half, padding.padding);
	it = std::copy(print_value.begin(), print_value.end(), it);
	return std::fill_n(it, right_half, padding.padding);
}
template<typename V, typename C>
struct int_padded_formatter : detail::padded_formatter<V, C>
{
	using detail::padded_formatter<V, C>::padded_formatter;
};
template<typename V, typename C = char>
int_padded_formatter<V, C> pad_int(const V & value, int num_padding, C padding = C('0'))
{
	return { value, num_padding, padding };
}
template<typename C, typename It, typename V>
format_it<C, It> format(format_it<C, It> it, const int_padded_formatter<V, C> & padding)
{
	if (padding.value < 0)
	{
		*it++ = '-';
		return it.print(pad_left(-padding.value, padding.num_padding - 1, padding.padding));
	}
	else return it.print(pad_left(padding.value, padding.num_padding, padding.padding));
}
}
