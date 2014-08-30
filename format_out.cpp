#include "format_out.hpp"
#include <iostream>

namespace fmt
{
format_it<char, std::ostreambuf_iterator<char> > cout{std::ostreambuf_iterator<char>(std::cout)};
format_it<char, std::ostreambuf_iterator<char> > cerr{std::ostreambuf_iterator<char>(std::cerr)};
format_it<char, std::ostreambuf_iterator<char> > clog{std::ostreambuf_iterator<char>(std::clog)};
format_it<wchar_t, std::ostreambuf_iterator<wchar_t> > wcout{std::ostreambuf_iterator<wchar_t>(std::wcout)};
format_it<wchar_t, std::ostreambuf_iterator<wchar_t> > wcerr{std::ostreambuf_iterator<wchar_t>(std::wcerr)};
format_it<wchar_t, std::ostreambuf_iterator<wchar_t> > wclog{std::ostreambuf_iterator<wchar_t>(std::wclog)};
}
