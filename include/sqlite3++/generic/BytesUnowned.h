#pragma once
#include "NoCopy.h"
#include "primitive_types.h"

#include <cstddef>

namespace sqlitepp
{

/// <summary>
/// This class is a thin wrapper around a pointer and length
/// The pointer is owned by SQLite, thus this instance must not have its
/// lifetime extended outside of row query
/// </summary>
template<typename TBytes>
class AnyBytesUnowned : public NoCopy
{
public:
  using byte_t = TBytes;
  using size_t = std::size_t;

  AnyBytesUnowned(const byte_t* ptr, size_t len)
    : ptr(ptr)
    , len(len)
  {
    static_assert(sizeof(byte_t) == 1, "This class only supports single byte types!");
  }

  AnyBytesUnowned()
    : ptr(nullptr)
    , len(0)
  {
    static_assert(sizeof(byte_t) == 1, "This class only supports single byte types!");
  }

  AnyBytesUnowned(AnyBytesUnowned&& other)
    : ptr(other.ptr)
    , len(other.len)
  {
    other.ptr = nullptr;
    other.len = 0;
  }

  AnyBytesUnowned& operator=(AnyBytesUnowned&& other)
  {
    ptr = other.ptr;
    len = other.len;
    other.len = 0;
    other.ptr = nullptr;
    return *this;
  }

  size_t size() const { return len; }
  const byte_t* data() const { return ptr; }
  byte_t operator[](size_t offset) const
  {
    return ptr[offset];
  }

protected:
  const byte_t* ptr;
  size_t len;
};

using BytesUnowned = AnyBytesUnowned<byte>;

}