#pragma once
#include "internal/RawStatement.h"
#include "generic/TemplateAssertFalse.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace sqlitepp
{

template <typename TBind>
struct BindTraits
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, const TBind& value) 
  {
    static_assert(TemplateAssertFalse<TBind>::value, "Cannot find correct conversion to bind this value.");
  }
};

template <>
struct BindTraits<std::string>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, const std::string& value) { binder.Bind(value.data(), value.size()); }
};

template <>
struct BindTraits<std::string_view>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, const std::string_view& value)
  {
    binder.Bind(value.data(), value.size());
  }
};

template <>
struct BindTraits<int>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, int value) { binder.Bind(value); }
};

template <>
struct BindTraits<std::int64_t>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, std::int64_t value) { binder.Bind(value); }
};

template <>
struct BindTraits<double>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, double value) { binder.Bind(value); }
};

template <>
struct BindTraits<const char*>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, const char* value) { binder.Bind(value); }
};

struct BindVoidData
{
  BindVoidData(const void* data, std::size_t size) : data(data), size(size) {}
  const void* data;
  const std::size_t size;
};

template <>
struct BindTraits<BindVoidData>
{
  static void BindValueToStatement(RawStatement::BindHelper& binder, const BindVoidData& value) { binder.Bind(value.data, value.size); }
};


}