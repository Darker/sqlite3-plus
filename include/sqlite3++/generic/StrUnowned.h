#pragma once
#include "BytesUnowned.h"

#include <string_view>
#include <cstring>

namespace sqlitepp
{

/// <summary>
/// This string class is a thin wrapper around a pointer and length
/// The pointer is owned by SQLite, thus this string must not have its
/// lifetime extended outside of row query
/// </summary>
class StrUnowned : public AnyBytesUnowned<char>
{
public:
  using size_t = std::size_t;
  using AnyBytesUnowned<char>::AnyBytesUnowned;


  // returns true if found, then the index assigned to second arg, if not found, value of index is undefined
  bool indexOf(std::string_view view, std::size_t& index) const;
};

inline bool StrUnowned::indexOf(std::string_view view, std::size_t& index) const
{
  if (view.size() > len)
    return false;
  if (view.size() == 0 && len != 0)
    return false;
  size_t max = len - view.size();
  for (size_t str_off = 0; str_off <= max; ++str_off)
  {
    int res = memcmp(view.data(), ptr + str_off, view.size());
    if (res == 0)
    {
      index = str_off;
      return true;
    }
  }
  return false;
}

}