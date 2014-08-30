#pragma once

#include "format_it.hpp"

namespace fmt
{
template<typename C = char>
struct format_overflow_callback
{
	static void (*callback)(const C * begin, const C * end);
};
template<typename C>
void (*format_overflow_callback<C>::callback)(const C * begin, const C * end) = nullptr;
namespace detail
{
template<typename C, typename A>
struct heap_format
{
	typedef std::vector<C, A> string_type;
	typedef std::deque<C, A> string_builder_type;

	heap_format()
	{
		new (&string_builder) string_builder_type();
	}
	~heap_format()
	{
		if (finished) finished_string.~string_type();
		else string_builder.~string_builder_type();
	}
	void push_back(C c)
	{
		string_builder.push_back(c);
	}
	template<typename It>
	void insert(It begin, It end)
	{
		string_builder.insert(string_builder.end(), begin, end);
	}
	void reserve(size_t)
	{
	}
	void finish()
	{
		string_builder.push_back(C('\0'));
		string_type string(std::make_move_iterator(string_builder.begin()), std::make_move_iterator(string_builder.end()));
		string_builder.~string_builder_type();
		static_assert(noexcept(string_type(std::move(string))), "don't want to unwind while this class is in an invalid state");
		new (&finished_string) string_type(std::move(string));
		finished = true;
	}
	const C * c_str() const
	{
		return finished_string.data();
	}
	C * begin()
	{
		return const_cast<C *>(const_cast<const heap_format &>(*this).begin());
	}
	C * end()
	{
		return const_cast<C *>(const_cast<const heap_format &>(*this).end());
	}
	const C * begin() const
	{
		return finished_string.data();
	}
	const C * end() const
	{
		return finished_string.data() + finished_string.size();
	}
	size_t size() const
	{
		return finished_string.size();
	}

private:
	bool finished = false;
	union
	{
		string_type finished_string;
		string_builder_type string_builder;
	};
};

template<typename C, typename FA>
struct stack_format_fallback
{
	explicit stack_format_fallback(size_t original_size)
		: original_size(original_size)
	{
	}
	~stack_format_fallback()
	{
		if (overflown) fallback.~heap_format();
	}

	// need __attribute__((noinline)) because gcc will inline this function
	// into stack_format_it::operator= which will cause that one's code to
	// bloat which makes gcc not inline that one. even though operator= has
	// the inline keyword and this function does not
	void overflow(C * buffer, C c) FORMAT_NO_INLINE
	{
		C * begin = buffer - this->original_size + 1;
		if (format_overflow_callback<C>::callback)
		{
			// null termination is not strictly required here but it's nice
			// for users
			*buffer = C('\0');
			format_overflow_callback<C>::callback(begin, buffer);
		}
		new (&fallback) heap_format<C, FA>();
		overflown = true;
		fallback.reserve(original_size * 2);
		fallback.insert(begin, buffer);
		fallback.push_back(c);
	}
	void push_back_overflow(C c) FORMAT_NO_INLINE
	{
		fallback.push_back(c);
	}
	void finish(C * buffer, size_t num_bytes_remaining)
	{
		if (did_overflow()) fallback.finish();
		else
		{
			*buffer = C('\0');
			num_bytes_written = original_size - num_bytes_remaining;
		}
	}
	bool did_overflow() const
	{
		return overflown;
	}
	const C * c_str(const C * buffer) const
	{
		if (did_overflow()) return fallback.c_str();
		else return buffer;
	}
	C * begin(C * buffer)
	{
		return const_cast<C *>(const_cast<const stack_format_fallback &>(*this).begin(buffer));
	}
	C * end(C * buffer)
	{
		return const_cast<C *>(const_cast<const stack_format_fallback &>(*this).end(buffer));
	}
	const C * begin(const C * buffer) const
	{
		if (did_overflow()) return fallback.begin();
		else return buffer;
	}
	const C * end(const C * buffer) const
	{
		if (did_overflow()) return fallback.end();
		else return buffer + num_bytes_written;
	}
	size_t size() const
	{
		if (did_overflow()) return fallback.size();
		else return num_bytes_written;
	}

private:
	bool overflown = false;
	union
	{
		char unused;
		heap_format<C, FA> fallback;
	};
	union
	{
		size_t original_size;
		size_t num_bytes_written;
	};
};

template<typename C, typename FA>
struct heap_format_it : std::iterator<std::output_iterator_tag, void, void, void, void>
{
	heap_format_it(heap_format<C, FA> & heap)
		: heap(&heap)
	{
	}
	heap_format_it & operator=(C c)
	{
		heap->push_back(c);
		return *this;
	}
	heap_format_it & operator*()
	{
		return *this;
	}
	heap_format_it & operator++()
	{
		return *this;
	}
	heap_format_it & operator++(int)
	{
		return *this;
	}
	void finish()
	{
		heap->finish();
	}
private:
	heap_format<C, FA> * heap;
};

template<typename C, typename FA>
struct stack_format_it : std::iterator<std::output_iterator_tag, void, void, void, void>
{
	stack_format_it(C * buffer, size_t buffer_size, stack_format_fallback<C, FA> & fallback)
		: buffer(buffer), buffer_size(buffer_size), fallback(&fallback)
	{
	}
	inline stack_format_it & operator=(C c)
	{
		if (buffer_size > 1)
		{
			*buffer++ = c;
			--buffer_size;
		}
		else if (buffer_size == 1)
		{
			// overflow if there's no space left for the nullterminator
			buffer_size = 0;
			fallback->overflow(buffer, c);
		}
		else
		{
			fallback->push_back_overflow(c);
		}
		return *this;
	}
	stack_format_it & operator*()
	{
		return *this;
	}
	stack_format_it & operator++()
	{
		return *this;
	}
	stack_format_it & operator++(int)
	{
		return *this;
	}

	void finish()
	{
		fallback->finish(buffer, buffer_size);
	}
private:
	C * buffer;
	size_t buffer_size;
	stack_format_fallback<C, FA> * fallback;
};
template<size_t Size, typename C, typename FA>
struct base_stack_format
{
	base_stack_format()
		: fallback(Size)
	{
	}

	C * begin()
	{
		return fallback.begin(buffer);
	}
	C * end()
	{
		return fallback.end(buffer);
	}
	const C * begin() const
	{
		return const_cast<base_stack_format &>(*this).begin();
	}
	const C * end() const
	{
		return const_cast<base_stack_format &>(*this).end();
	}
	size_t size() const
	{
		return fallback.size();
	}
	const C * c_str() const
	{
		return fallback.c_str(buffer);
	}
	bool did_overflow() const
	{
		return fallback.did_overflow;
	}
	bool operator==(const char * str) const
	{
		return strcmp(str, c_str()) == 0;
	}
protected:
	format_it<C, stack_format_it<C, FA> > build_iterator()
	{
		return { stack_format_it<C, FA>(buffer, Size, fallback) };
	}

	detail::stack_format_fallback<C, FA> fallback;
	C buffer[Size];
};
template<typename C, typename FA>
struct base_stack_format<0, C, FA>
{
	base_stack_format()
	{
	}

	C * begin()
	{
		return heap.begin();
	}
	C * end()
	{
		return heap.end();
	}
	const C * begin() const
	{
		return heap.begin();
	}
	const C * end() const
	{
		return heap.end();
	}
	size_t size() const
	{
		return heap.size();
	}
	const C * c_str() const
	{
		return heap.c_str();
	}
	bool operator==(const char * str) const
	{
		return strcmp(str, c_str()) == 0;
	}

protected:
	format_it<C, heap_format_it<C, FA> > build_iterator()
	{
		return { heap_format_it<C, FA>(heap) };
	}
	heap_format<C, FA> heap;
};
template<size_t Size, typename C, typename FA>
bool operator==(const char * str, const base_stack_format<Size, C, FA> & printer)
{
	return printer == str;
}
}

template<size_t Size, typename C = char, typename FA = std::allocator<C> >
struct stack_format_func : detail::base_stack_format<Size, C, FA>
{
	template<typename Func>
	stack_format_func(Func && func)
	{
		auto out = this->build_iterator();
		out = func(out);
		out.it().finish();
	}
};
template<typename Traits, size_t Size, typename C, typename FA>
std::basic_ostream<C, Traits> & operator<<(std::basic_ostream<C, Traits> & lhs, const stack_format_func<Size, C, FA> & printer)
{
	lhs.write(printer.c_str(), printer.size());
	return lhs;
}

template<size_t Size, typename C = char, typename FA = std::allocator<C> >
struct stack_format : detail::base_stack_format<Size, C, FA>
{
	template<typename... Args>
	stack_format(Args &&... args)
	{
		auto out = this->build_iterator();
		out.format(std::forward<Args>(args)...);
		out.it().finish();
	}
};
template<typename Traits, size_t Size, typename C, typename FA>
std::basic_ostream<C, Traits> & operator<<(std::basic_ostream<C, Traits> & lhs, const stack_format<Size, C, FA> & printer)
{
	lhs.write(printer.c_str(), printer.size());
	return lhs;
}

template<size_t Size, typename C = char, typename FA = std::allocator<C> >
struct stack_print : detail::base_stack_format<Size, C, FA>
{
	template<typename... Args>
	stack_print(const Args &... args)
	{
		auto out = this->build_iterator();
		out.print(args...);
		out.it().finish();
	}
};
template<typename Traits, size_t Size, typename C, typename FA>
std::basic_ostream<C, Traits> & operator<<(std::basic_ostream<C, Traits> & lhs, const stack_print<Size, C, FA> & printer)
{
	lhs.write(printer.c_str(), printer.size());
	return lhs;
}
}
