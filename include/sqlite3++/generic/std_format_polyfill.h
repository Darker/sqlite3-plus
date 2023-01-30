#pragma once

//#define __clang__ 1
//#define __clang_major__ 12
//#define __clang_minor__ 0
//#define __clang_patchlevel__ 8

#if _MSC_VER >= 1930
#include <format>
#else
#include <string>
namespace std
{
template<typename ...Vargs>
string format(const string& f, Vargs... args) { return ""; }
}

#endif
