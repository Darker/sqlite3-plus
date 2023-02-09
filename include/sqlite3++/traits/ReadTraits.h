#pragma once
#include "../internal/RawStatement.h"
#include "../generic/TemplateAssertFalse.h"
#include "../generic/StrUnowned.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace sqlitepp
{

template <typename TRead>
struct ReadTraits
{
  static TRead ReadFromStatement(RawStatement::ReadHelper& reader)
  {
    static_assert(TemplateAssertFalse<TRead>::value, "Cannot find correct conversion to read from this column.");
  }
};


template<>
struct ReadTraits<int>
{
  static int ReadFromStatement(RawStatement::ReadHelper& reader)
  {
    return reader.ReadInt();
  }
};

template<>
struct ReadTraits<std::int64_t>
{
  static std::int64_t ReadFromStatement(RawStatement::ReadHelper& reader)
  {
    return reader.ReadLongInt();
  }
};

template<>
struct ReadTraits<double>
{
  static double ReadFromStatement(RawStatement::ReadHelper& reader)
  {
    return reader.ReadDouble();
  }
};

template<>
struct ReadTraits<StrUnowned>
{
  static StrUnowned ReadFromStatement(RawStatement::ReadHelper& reader)
  {
    return reader.ReadString();
  }
};

template<>
struct ReadTraits<BytesUnowned>
{
  static BytesUnowned ReadFromStatement(RawStatement::ReadHelper& reader)
  {
    return reader.ReadBlob();
  }
};

}
