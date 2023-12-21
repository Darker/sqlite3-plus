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
#include <set>
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
    else if (*c == '\\')
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
  std::regex wordRegex(R"RE((^[^a-zA-Z_]|[^\w]))RE");
  std::smatch match;
  if (std::regex_search(keyName, match, wordRegex))
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
    return prefix + escapeKey(std::string{ suffix }, true);
  }
}

auto mergePrefix(const std::string& prefix, unsigned int arrayIndex)
{
  return prefix + std::string{ "[" } + std::to_string(arrayIndex) + "]";;
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

//  https://stackoverflow.com/a/25958302/607407
template <typename T, typename Tuple>
struct tuple_has_type;

template <typename T>
struct tuple_has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct tuple_has_type<T, std::tuple<U, Ts...>> : tuple_has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct tuple_has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename TSearch, typename TTuple>
using tuple_contains_type = typename tuple_has_type<TSearch, TTuple>::type;

template <typename TSearch, typename TTuple>
constexpr auto tuple_contains_type_v = tuple_contains_type<TSearch, TTuple>::value;

class wrong_something : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class wrong_type : public wrong_something
{
public:
  using wrong_something::wrong_something;

  static wrong_type create(std::string_view keyName, std::string_view expectedType, std::string_view realType)
  {
    return wrong_type(
        std::string{ "at \"" } 
      + std::string{ keyName } 
      + "\" is not " 
      + std::string{ expectedType }
      + " but "
      + std::string{ realType }
    );
  }
  template <typename TExpected>
  static wrong_type create(std::string_view keyName, std::string_view realType)
  {
    return wrong_type(
      std::string{ "at \"" }
      + std::string{ keyName }
      + "\" is not "
      + TypeName<TExpected>::Get()
      + " but "
      + std::string{ realType }
    );
  }
};

class wrong_key : public wrong_something
{
  using wrong_something::wrong_something;
};

class wrong_value : public wrong_something
{
  using wrong_something::wrong_something;
};


namespace special_check
{
struct special_check_flag {};
struct is_null final : special_check_flag {};
struct is_set final : special_check_flag {};

template <typename TNum>
struct is_num_in_range final : special_check_flag {
  constexpr is_num_in_range(TNum minVal, TNum maxVal)
    : min{minVal}
    , max{maxVal}
  {}

  TNum min;
  TNum max;
};

template <typename T>
struct no_overload_error : std::false_type {};

}

template <typename TStr>
constexpr bool isSomeString = std::is_same_v<const char*, TStr> || std::is_same_v<std::string, TStr>;

template <typename TNullType>
constexpr bool representsNull = std::is_same_v<std::nullopt_t, TNullType>
|| std::is_same_v<std::nullptr_t, TNullType>
|| std::is_same_v<special_check::is_null, TNullType>;

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

template <typename special_check_t>
bool checkSpecial(const json& obj, special_check_t expected, const std::string& keyPath = "?")
{
  static_assert(special_check::no_overload_error<special_check_t>::value, "Special check not implemented.");
}

template <typename TNum>
bool checkSpecial(const json& obj, special_check::is_num_in_range<TNum> rangeCheck, const std::string& keyPath = "?")
{
  if (isOfPrimitiveType<TNum>(obj))
  {
    const TNum num = obj.get<TNum>();
    if (num < rangeCheck.min || num > rangeCheck.max)
    {
#ifdef __cpp_lib_format
      throw wrong_value(std::format("at \"{}\" not in allowed range {} <= (val:{}) <= {}", keyPath, rangeCheck.min, num, rangeCheck.max));
#else
      throw wrong_value("wrong value");
#endif
      return false;
    }
  }
  else
  {
    throw wrong_type::create<TNum>(keyPath, obj.type_name());
    return false;
  }
  return true;
}


//template <>
//struct TypeName<is_null>
//{
//  static constexpr const char* Get()
//  {
//    return "json_is_null";
//  }
//};
template <typename TValue>
bool checkJSONValue(const json& obj, TValue expected, const std::string& keyPath = "?");

template <typename TValue>
struct pair
{
  using value_t = TValue;

  // needed for gcc
  constexpr pair(const std::string& key, TValue&& expected_value)
    : key(key)
    , value{std::forward<TValue>(expected_value)}
  { }

  std::string key;
  bool keyIsArray = false;
  TValue value;

  template <typename T>
  struct no_overload_error : std::false_type {};

  bool check(const json& obj, const std::string& prefix) const
  {
    //static_assert(no_overload_error<TValue>::value, "Cannot check this data type.");
    //return false;
    auto const checkedKey = mergePrefix(prefix, key);
    return checkJSONValue(obj, value, checkedKey);
  }
};

struct nested_obj
{
  template <typename ... Values>
  constexpr nested_obj(pair<Values> ... pairs);

  std::function<bool(const json& obj, const std::string& prefix)> nested_real_check;
  // using check_t = decltype([](const json& obj, const std::string& prefix) { return true; });
  // check_t nested_real_check;
};

struct nested_array
{
  template <typename ... Values>
  constexpr nested_array(Values... pairs);

  std::function<bool(const json& obj, const std::string& prefix)> nested_array_check;
};

template <typename TValue>
bool checkJSONValue(const json& obj, TValue expected, const std::string& keyPath)
{
  if constexpr (std::is_same_v<nested_obj, TValue>)
  {
    if (!expected.nested_real_check(obj, keyPath))
    {
      return false;
    }
  }
  else if constexpr (std::is_same_v<nested_array, TValue>)
  {
    if (!expected.nested_array_check(obj, keyPath))
    {
      return false;
    }
  }
  else if constexpr (std::is_base_of_v<special_check::special_check_flag, TValue>)
  {
    return checkSpecial(obj, expected, keyPath);
  }
  else if (isOfPrimitiveType<TValue>(obj))
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

template<>
bool pair<nested_array>::check(const json& obj, const std::string& prefix) const
{
  auto const checkedKey = mergePrefix(prefix, key);
  return value.nested_array_check(obj, prefix);
}


template <typename TFirst, typename ... Values>
bool check_object_with_keys(const json& obj, const std::string& prefix, std::set<std::string>& checked_keys, pair<TFirst> first, pair<Values> ... pairs)
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
  std::cout << "Checking " << mergePrefix(prefix, first.key) << "\n";
  if (!first.check(obj[first.key], prefix))
  {
    return false;
  }
  else
  {
    std::cout << "Adding " << mergePrefix(prefix, first.key) << "\n";
    checked_keys.insert(first.key);
  }

  if constexpr (sizeof...(pairs) > 0)
  {
    return check_object_with_keys(obj, prefix, checked_keys, pairs...);
  }
  return true;
}

template <typename TFirst, typename ... Values>
bool check_object_values(const json& obj, const std::string& prefix, pair<TFirst> first, pair<Values> ... pairs)
{
  std::set<std::string> checkedKeys;
  const bool result = check_object_with_keys(obj, prefix, checkedKeys, first, pairs...);
  if (result)
  {
    // check if there are no additional keys
    for (const auto& [key, value] : obj.items())
    {
      if (checkedKeys.find(key) == checkedKeys.end())
      {
        const auto fullName = mergePrefix(prefix, key);
        //std::cout << (std::string{ "at \"" } + fullName + "\" unexpected entry (" + value.type_name() + ")") + "\n";
        throw wrong_key(std::string{"at \""} + fullName + "\" unexpected entry (" + value.type_name() + ")");
        return false;
      }
    }
  }
  return result;
}

template <typename TFirst, typename ... Values>
bool check_array_values(const json& obj, const std::string& prefix, int index, TFirst first, Values ... values)
{
  if (!obj.is_array())
  {
#ifdef __cpp_lib_format
    throw wrong_type(std::format("at \"{}\" is not an array", prefix));
#else
    throw wrong_type(prefix + "not an object");
#endif
  }

  const auto currentName = mergePrefix(prefix, index);

  if (obj.size() <= index)
  {
#ifdef __cpp_lib_format
    throw wrong_key(std::format("at \"{}\" not defined", currentName));
#else
    throw wrong_key(currentName + "not defined");
#endif
  }

  if (!checkJSONValue(obj[index], first, currentName))
  {
    return false;
  }

  if constexpr (sizeof...(values) > 0)
  {
    return check_array_values(obj, prefix, index+1, values...);
  }
  else if(obj.size() > index + 1)
  {
    const auto nextName = mergePrefix(prefix, index + 1);
#ifdef __cpp_lib_format
    throw wrong_key(std::format("at \"{}\" array has an unexpected item", nextName));
#else
    throw wrong_key(nextName + " array has an unexpected item");
#endif
  }
  return true;
}

template <typename ... Values>
constexpr nested_obj::nested_obj(pair<Values> ... pairs)
  : nested_real_check{ [pairs...](const json& obj, const std::string& prefix) { return check_object_values(obj, prefix, pairs...); } }
{}

template <typename ... Values>
constexpr nested_array::nested_array(Values ... values)
  : nested_array_check{ [values...](const json& obj, const std::string& prefix) { return check_array_values(obj, prefix, values...); } }
{}

template <typename TJsonData>
constexpr auto deriveJSONCheck(TJsonData&& innerCheck)
{
  return nested_obj{
    pair{ "success", true },
    pair{ "data", std::forward<TJsonData>(innerCheck) }
  };
}

//template <uint64_t resultCode>
//constexpr auto checkResult = deriveJSONCheck(nested_obj{ pair{"code", resultCode} });

int main()
{
  try
  {
    const json real_json = json::parse(R"json(
        {"boolval": true,
         "numval":19, 
         "data":{
            "in bool":false,
            "in int": 65,
            "expected null": null,
            "ar":[1,2,3, {"res":true}]
         }
        }
    )json");

    //check_object_values(real_json, "", pair{"boolval", true}, pair{"numval", 22});

    check_object_values(real_json, "",
      pair{ "boolval", true },
      pair{ "numval", special_check::is_num_in_range{10, 20} },
      pair{ "data", nested_obj{
        pair{"in bool", false},
        pair{"in int", 65},
        pair{"expected null", nullptr},
        pair{"ar", nested_array{
          1,
          2,
          3,
          nested_obj{
            pair{"res", true},
          }
        } }
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
    const json real_json = json::parse(R"json(
        {
          "success": true,
          "data": {"code": 42}
        }
    )json");

    //check_object_values(real_json, "", pair{"boolval", true}, pair{"numval", 22});

    checkJSONValue(real_json, deriveJSONCheck(nested_obj{ pair{"code", 41} }), "");
  }
  catch (const wrong_something& exc)
  {
    std::cout << "FAIL: " << exc.what() << std::endl;
    return 1;
  }
  return 0;
}