#ifndef DISABLE_GTEST
#include <gtest/gtest.h>
#endif

#include "stack_format.hpp"
#include "format_helpers.hpp"
#include "format_stl.hpp"
#include "format_out.hpp"
#include <string>
#include <iterator>

struct print_through_ostream
{
	int id = 0;
	std::string name = "hank";
};

std::ostream & operator<<(std::ostream & lhs, const print_through_ostream & rhs)
{
	return lhs << "id: " << rhs.id << ", name: " << rhs.name;
}

struct print_through_format_it
{
	int id = 0;
	std::string name = "hank";
};
template<typename C, typename It>
fmt::format_it<C, It> format(fmt::format_it<C, It> it, const print_through_format_it & self)
{
	return it.format("id: %0, name: %1", self.id, self.name);
	// alternatively:
	//return it.printpacked("id: ", self.id, ", name: ", self.name);
	// alternatively:
	// *it++ = "id: ";
	// *it++ = self.id;
	// *it++ = ", name: ";
	// return *it++ = self.name;
}

std::string print_both(const print_through_ostream & first, const print_through_format_it & second)
{
	std::string out;
	auto it = fmt::make_format_it(std::back_inserter(out));
	it.print_separated('\n', first, second);
	return out;
}

int main(int argc, char * argv[])
{
	// format "100, 0.5" into 1024 bytes of stack memory
	fmt::stack_format<1024> format("%0, %1", 100, 0.5f);

	// print "Hello World" into 1024 bytes of stack memory
	fmt::stack_print<1024> print("Hello", "World");

	// prints 100, 0.5\nHello World,
	auto format_to_cout = fmt::make_format_it(std::ostreambuf_iterator<char>(std::cout));
	*format_to_cout++ = format;
	*format_to_cout++ = '\n';
	*format_to_cout++ = print;
	*format_to_cout++ = '\n';

	std::vector<int> numbers = { 1, 10, 100 };
	std::string a_string;
	auto format_to_string = fmt::make_format_it(std::back_inserter(a_string));
	// writes "{ 1, 10, 100 }" into the string
	*format_to_string++ = numbers;
	// also possible:
	format_to_string.format("\n%0: %1\n", numbers.size(), numbers);
	// that wrote "\n3: { 1, 10, 100 }\n" to the string. so the string is now
	// {1, 10, 100 }
	// 3: { 1, 10, 100 }
	//
	fmt::cout.print(a_string);

	// prints "1, a, 64"
	std::transform(numbers.begin(), numbers.end(), fmt::with_separator(format_to_cout, ", "), &fmt::hex<int>);
	*format_to_cout++ = '\n';
	// prints "{ 1, 10, 100 }"
	*format_to_cout++ = numbers;
	*format_to_cout++ = '\n';

	// prints "1 2 3"
	fmt::cout.print(1, 2, 3).print('\n');
	// prints "1, 2, 3"
	fmt::cout.print_separated(", ", 1, 2, 3).print('\n');
	// prints "123"
	fmt::cout.printpacked(1, 2, 3).print('\n');

	// prints "{ 1, 10, 100 }"
	fmt::cout.print(numbers);
	// prints "110100"
	std::copy(numbers.begin(), numbers.end(), fmt::cout).print('\n');
	// prints "1, 10, 100"
	std::copy(numbers.begin(), numbers.end(), fmt::with_separator(fmt::cout, "\n")).it.print('\n');
	// prints "1, a, 64"
	std::transform(numbers.begin(), numbers.end(), fmt::with_separator(fmt::cout, "\n"), &fmt::hex<int>).it.print('\n');


	fmt::cout.print(print_both(print_through_ostream(), print_through_format_it())).print('\n');

#ifndef DISABLE_GTEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif
}

