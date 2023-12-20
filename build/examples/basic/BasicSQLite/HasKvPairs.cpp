// required
#include <string>
#include <functional>
#include <type_traits>
// for mock implementation
#include <map>
#include <any>
#include <stdexcept>
#include <memory>
// for debug
#include <iostream>
#include <regex>
#ifdef __cpp_lib_format
#include <format>
#endif

#define JSON_NO_IO
#include "./json.h"
using json = nlohmann::json;

namespace nlohmann
{
  constexpr const char* type_name(const detail::value_t type) noexcept
  {
    using value_t = detail::value_t;
    switch (type)
    {
    case value_t::null:
      return "null";
    case value_t::object:
      return "object";
    case value_t::array:
      return "array";
    case value_t::string:
      return "string";
    case value_t::boolean:
      return "boolean";
    case value_t::binary:
      return "binary";
    case value_t::discarded:
      return "discarded";
    case value_t::number_integer:
    case value_t::number_unsigned:
    case value_t::number_float:
    default:
      return "number";
    }
  }
}

class wrong_something : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class wrong_type : public wrong_something
{
  using wrong_something::wrong_something;
};

class wrong_key : public wrong_something
{
  using wrong_something::wrong_something;
};

class wrong_value : public wrong_something
{
  using wrong_something::wrong_something;
};



struct fake_json
{
  static const fake_json& dummy()
  {
    static fake_json f;
    return f;
  }

  using ptr = std::unique_ptr<fake_json>;
  struct nested_index
  {
    std::size_t index;
  };

  fake_json() {}
  fake_json(const fake_json& other) = delete;
  fake_json& operator=(const fake_json& other) = delete;

  template <typename TValue>
  void set(const std::string& key, TValue&& value)
  {
    auto iter = values.find(key);
    if (iter != values.end())
    {
      iter->second.emplace<TValue>(value);
    }
    else
    {
      values.insert({ key, value });
    }
  }

  template <typename T>
  bool check_if_equals(const std::string& key, T&& value) const
  {
    const auto iter = values.find(key);
    if (iter != values.cend())
    {
      try
      {
        const auto cast_value = std::any_cast<T>(iter->second);
        bool same = value == cast_value;
        if (!same)
        {
#ifdef __cpp_lib_format
          throw wrong_value(std::format("at \"{}\" {} != {}", key, (T)cast_value, (T&&)value));
#else
          throw wrong_value("wrong value");
#endif

          return false;
        }
      }
      catch (const std::bad_any_cast& bad)
      {
        throw wrong_type(bad.what());
        return false;
      }
    }
    else
    {
#ifdef __cpp_lib_format
      throw wrong_key(std::format("Key not found: {}", key));
#else
      throw wrong_value("key not found");
#endif

    }
    return true;
  }

  template <typename T>
  bool check_if_is(const std::string& key) const
  {
    const auto iter = values.find(key);
    if (iter != values.cend())
    {
      try
      {
        const auto cast_value = std::any_cast<T>(iter->second);
      }
      catch (const std::bad_any_cast& bad)
      {
        throw wrong_type(bad.what());
        return false;
      }
    }
    else
    {
#ifdef __cpp_lib_format
      throw wrong_key(std::format("Key not found: {}", key));
#else
      throw wrong_key("key not found");
#endif

    }
    return true;
  }

  const fake_json& get_nested(const std::string& key) const
  {
    const auto iter = values.find(key);
    if (iter != values.cend())
    {
      try
      {
        const auto index = std::any_cast<nested_index>(iter->second);
        if (index.index < nested.size())
        {
          return *nested[index.index];
        }
        else
        {
          throw wrong_value("Invalid nested index");
          return dummy();
        }
      }
      catch (const std::bad_any_cast& bad)
      {
        throw wrong_type(bad.what());
        return dummy();
      }
    }
    else
    {
#ifdef __cpp_lib_format
      throw wrong_key(std::format("Key not found: {}", key));
#else
      throw wrong_key("key not found");
#endif
      return dummy();
    }
  }

  fake_json& get_nested(const std::string& key)
  {
    return const_cast<fake_json&>((const_cast<const fake_json*>(this)->get_nested(key)));
  }

  fake_json& nest(const std::string& key)
  {
    if (values.find(key) == values.cend())
    {
      nested.push_back(std::move(ptr{ new fake_json() }));
      values.emplace(std::make_pair(key, std::make_any<nested_index>(nested_index{ nested.size() - 1 })));
      return *nested[nested.size() - 1];
    }
    else
    {
      return get_nested(key);
    }
  }

  private:
    std::map<std::string, std::any> values;
    std::vector<ptr> nested;
};

struct is_null final {};
struct is_set final {};

template <typename TValue>
struct pair
{
  using value_t = TValue;

  // needed for gcc
  pair(const std::string& key, TValue&& expected_value)
    : key(key)
    , value{std::forward<TValue>(expected_value)}
  { }

  std::string key;
  bool keyIsArray = false;
  TValue value;

  template <typename T>
  struct no_overload_error : std::false_type {};

  bool check(const fake_json& obj) const
  {
    static_assert(no_overload_error<TValue>::value, "Cannot check this data type.");
    return false;
  }

  bool check(const json& obj, const std::string& prefix) const
  {
    static_assert(no_overload_error<TValue>::value, "Cannot check this data type.");
    return false;
  }
};

struct nested_obj
{
  template <typename ... Values>
  nested_obj(pair<Values> ... pairs);

  std::function<bool(const fake_json& obj)> nested_check;
  std::function<bool(const json& obj, const std::string& prefix)> nested_real_check;
};

template<>
bool pair<bool>::check(const fake_json& obj) const
{
  return obj.check_if_equals(key, value);
}

template<>
bool pair<int>::check(const fake_json& obj) const
{
  return obj.check_if_equals(key, value);
}

template<>
bool pair<nested_obj>::check(const fake_json& obj) const
{
  return value.nested_check(obj.get_nested(key));
}

template <typename TFirst, typename ... Values>
bool check_object_values(const fake_json& obj, pair<TFirst> first, pair<Values> ... pairs)
{
  if (!first.check(obj))
  {
    return false;
  }

  if constexpr (sizeof...(pairs) > 0)
  {
    return check_object_values(obj, pairs...);
  }
  return true;
}
// intended for escaping object keys and string values
auto wrapSpecialChars(const std::string& input)
{
  std::ostringstream o;
  bool changed = false;
  for (auto c = input.cbegin(); c != input.cend(); c++)
  {
    if (*c == '"')
    {
      o << "\\\"";
      changed = true;
    }
    if (*c == '\\')
    {
      o << "\\\\";
      changed = true;
    }
    else if ('\x00' <= *c && *c <= '\x1f')
    {
      o << "\\u"
        << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
      changed = true;
    }
    else
    {
      o << *c;
    }
  }
  if (changed)
  {
    return o.str();
  }
  else
  {
    return input;
  }
}

auto escapeKey(const std::string& keyName, bool addDotIfNotEscaped = false)
{
  std::regex wordRegex(R"RE((?:^[^a-zA-Z_]|[^\w]))RE");
  std::smatch match;
  if (std::regex_match(keyName, match, wordRegex))
  {
    return std::string{ "[\"" + wrapSpecialChars(keyName) + "\"]" };
  }
  else if (addDotIfNotEscaped)
  {
    return "." + keyName;
  }
  else
  {
    return keyName;
  }
}

auto mergePrefix(const std::string& prefix, std::string_view suffix)
{
  if (prefix.empty())
  {
    return escapeKey(std::string{ suffix });
  }
  else
  {
    return escapeKey(prefix) + escapeKey(std::string{ suffix }, true);
  }
}

template <typename TStr>
constexpr bool isSomeString = std::is_same_v<const char*, TStr> || std::is_same_v<std::string, TStr>;

template <typename TNullType>
constexpr bool representsNull = std::is_same_v<std::nullopt_t, TNullType>
                      || std::is_same_v<std::nullptr_t, TNullType>
                      || std::is_same_v<is_null, TNullType>;


template <typename ValueType>
bool isOfPrimitiveType(const json& obj)
{
  if (!obj.is_primitive())
  {
    return false;
  }
  if constexpr (std::is_same_v<bool, ValueType>)
  {
    return obj.is_boolean();
  }
  else if constexpr (std::is_arithmetic_v<ValueType>)
  {
    return obj.is_number();
  }
  else if constexpr (isSomeString<ValueType>)
  {
    return obj.is_string();
  }
  else if constexpr (representsNull<ValueType>)
  {
    return obj.is_null();
  }
  else
  {
    return false;
  }
}

/// https://stackoverflow.com/a/4485051/607407
// default implementation
template <typename T>
struct TypeName
{
  static constexpr const char* Get()
  {
    return typeid(T).name();
  }
};

//template <>
//struct TypeName<is_null>
//{
//  static constexpr const char* Get()
//  {
//    return "json_is_null";
//  }
//};


template <typename TValue>
bool checkJSONValue(const json& obj, TValue expected, const std::string& keyPath = "?")
{
  if (isOfPrimitiveType<TValue>(obj))
  {
    if constexpr (representsNull<TValue>)
    {
      return true;
    }
    else
    {
      const TValue val = obj.get<TValue>();
      if (val != expected)
      {
#ifdef __cpp_lib_format
        throw wrong_value(std::format("at \"{}\" {} != {}", keyPath, val, expected));
#else
        throw wrong_value("wrong value");
#endif
        return false;
      }
    }
  }
  else
  {
#ifdef __cpp_lib_format
    throw wrong_type(std::format("at \"{}\" is not {} but {}", keyPath, TypeName<TValue>::Get(), obj.type_name()));
#else
    throw wrong_type("wrong type");
#endif
    return false;
  }
  return true;
}

template<>
bool pair<is_null>::check(const json& obj, const std::string& prefix) const
{
  auto const checkedKey = mergePrefix(prefix, key);
  return checkJSONValue(obj, value, checkedKey);
}

template<>
bool pair<bool>::check(const json& obj, const std::string& prefix) const
{
  auto const checkedKey = mergePrefix(prefix, key);
  return checkJSONValue(obj, value, checkedKey);
}

template<>
bool pair<int>::check(const json& obj, const std::string& prefix) const
{
  auto const checkedKey = mergePrefix(prefix, key);
  return checkJSONValue(obj, value, checkedKey);
}

template<>
bool pair<nested_obj>::check(const json& obj, const std::string& prefix) const
{
  auto const checkedKey = mergePrefix(prefix, key);
  if (obj.is_object())
  {
    return value.nested_real_check(obj, checkedKey);
  }
  else
  {
#ifdef __cpp_lib_format
    throw wrong_type(std::format("at \"{}\" is not an object", checkedKey));
#else
    throw wrong_type("wrong type");
#endif
    return false;
  }
  return true;
}

template <typename TFirst, typename ... Values>
bool check_object_values(const json& obj, const std::string& prefix, pair<TFirst> first, pair<Values> ... pairs)
{
  if (!obj.is_object())
  {
#ifdef __cpp_lib_format
    throw wrong_type(std::format("at \"{}\" is not an object", prefix));
#else
    throw wrong_type(prefix + "not an object");
#endif
  }
  if (!obj.contains(first.key))
  {
#ifdef __cpp_lib_format
    throw wrong_key(std::format("at \"{}\" not defined", mergePrefix(prefix, first.key)));
#else
    throw wrong_key(checkedKey + "not defined");
#endif
  }
  if (!first.check(obj[first.key], prefix))
  {
    return false;
  }

  if constexpr (sizeof...(pairs) > 0)
  {
    return check_object_values(obj, prefix, pairs...);
  }
  return true;
}

template <typename ... Values>
nested_obj::nested_obj(pair<Values> ... pairs)
  : nested_check{ [pairs...](const fake_json& obj) { return check_object_values(obj, pairs...); } }
  , nested_real_check{ [pairs...](const json& obj, const std::string& prefix) { return check_object_values(obj, prefix, pairs...); } }
{}

int main()
{
  try
  {
    fake_json fake;
    fake.set("bool", true);
    fake.set("int", 22);

    check_object_values(fake, pair{ "bool", true }, pair{ "int", 22 });


    auto& data = fake.nest("data");
    data.set("in bool", false);
    data.set("in int", 62);

    check_object_values(fake,
      pair{ "bool", true },
      pair{ "int", 22 },
      pair{ "data", nested_obj{
        pair{"in bool", false},
        pair{"in int", 62}
      } }
    );
  }
  catch (const wrong_something& exc)
  {
    std::cout << "FAIL: " << exc.what() << std::endl;
    return 1;
  }

  try
  {
    json real_json = json::parse(R"json(
        {"boolval": true,
         "numval":22, 
         "data":{"in bool":false, "in int": 65, "expected \"null\"": null}}
    )json");

    check_object_values(real_json, "", pair{"boolval", true}, pair{"numval", 22});

    check_object_values(real_json, "",
      pair{ "boolval", true },
      pair{ "numval", 22 },
      pair{ "data", nested_obj{
        pair{"in bool", false},
        pair{"in int", 62},
        pair{"expected \"null\"", nullptr}
      } }
    );
  }
  catch (const wrong_something& exc)
  {
    std::cout << "FAIL: " << exc.what() << std::endl;
    return 1;
  }
  return 0;
}