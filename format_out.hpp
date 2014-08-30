#pragma once

#include "format_it.hpp"
#include <ostream>
#include <iterator>

namespace fmt
{
extern format_it<char, std::ostreambuf_iterator<char> > cout;
extern format_it<char, std::ostreambuf_iterator<char> > cerr;
extern format_it<char, std::ostreambuf_iterator<char> > clog;
extern format_it<wchar_t, std::ostreambuf_iterator<wchar_t> > wcout;
extern format_it<wchar_t, std::ostreambuf_iterator<wchar_t> > wcerr;
extern format_it<wchar_t, std::ostreambuf_iterator<wchar_t> > wclog;
}
