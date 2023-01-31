#pragma once
#include <string>
#include <memory>

#include "internal/RawStatement.h"
#include "traits/ReadTraits.h"
#include "traits/BindTraits.h"
#include "generic/tuple_to_args.h"

#include <functional>
#include <tuple>

namespace sqlitepp
{

class Database;

template <typename... TResults>
class Statement
{
  friend class Database;
public:

  using RowHandler = std::function<bool(TResults...)>;
  //using RowHandler = std::function<bool(std::tuple<TResults...>&)>;

  Statement(const std::string& query);

  void Init(Database* db);

  //! Executes the statement using given values
  template <typename... TValRest>
  void execute(const RowHandler& handler, TValRest... values);

protected:
  //! Binds a value to the statement at a current offset
  //! It is your responsibility to bind the values in the correct order
  template <typename TValue>
  void bindValue(const TValue& value);

  //! Binds values to a prepared statement, after this the statement may be executed
  template <typename... TValRest>
  void bindValues(TValRest... values);

  //! Execute the statement and pass each result row to the row handler
  void executePrepared(const RowHandler& rowHandler);

  //! Provides pointer to the implementation of raw access. Use with caution, or ideally not at all
  RawStatement& getRaw() { return *raw; }
  const RawStatement& getRaw() const { return *raw; }
private:
  Database* db;
  std::unique_ptr<RawStatement> raw = nullptr;
  bool isValid = true;
};

template<typename ...TResults>
Statement<TResults...>::Statement(const std::string& query)
  : db(nullptr)
  , raw(new RawStatement(query))
{
}
template<typename ...TResults>
void Statement<TResults...>::Init(Database* db)
{
  raw->SetDb(db);
}
//template <typename... TResults>
//void Statement<TResults...>::Statement(const std::string& query, Database* db)
//{
//
//}

template <typename... TResults>
template <typename TValue>
void Statement<TResults...>::bindValue(const TValue& value)
{
  BindTraits<TValue>::BindValueToStatement(raw->getBinder(), value);
}

template <typename... TResults>
template <typename... TValRest>
void Statement<TResults...>::bindValues(TValRest... values)
{
  //bindValue(std::forward<TValFirst>(value));
  (bindValue(std::forward<TValRest>(values)), ...);
}

template <typename... TResults>
template <typename... TValRest>
void Statement<TResults...>::execute(const RowHandler& handler, TValRest... values)
{
  raw->Init();
  bindValues(values...);
  executePrepared(handler);
}
//
//template <typename... TReadTypes>
//auto readValues(RawStatement::ReadHelper& reader)
//{
//  return (ReadTraits<TResults>::ReadFromStatement(reader)...);
//}
//
//template <typename... TReadTypes>
//auto readValues(RawStatement::ReadHelper& reader)
//{
//  return (ReadTraits<TResults>::ReadFromStatement(reader)...);
//}

template <typename F, typename Tuple>
void call(F f, Tuple&& t)
{
  typedef typename std::decay<Tuple>::type ttype;
  detail::call_impl<F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}

template <typename... TResults>
void Statement<TResults...>::executePrepared(const RowHandler& handler)
{

  //((ReadTraits<TResults...>::ReadFromStatement()), ...);
  raw->Execute([handler](RawStatement::ReadHelper& reader) -> bool
  {
    std::tuple<TResults...> results;

    std::apply
    (
      [&reader](TResults&... tupleArgs)
      {
        ((tupleArgs = ReadTraits<TResults>::ReadFromStatement(reader)), ...);
      }, results
    );
    return call_with_tuple(handler, std::move(results));
    //return handler(results);
    //return handler(ReadTraits<TResults>::ReadFromStatement(reader)...);
    
    //return handler(ReadTraits<double>::ReadFromStatement(reader), ReadTraits<int>::ReadFromStatement(reader));
    //return handler(readValues<TResults>(reader)...);
  });
}

template <>
void Statement<>::executePrepared(const RowHandler& handler)
{
  raw->Execute([handler](RawStatement::ReadHelper& reader) -> bool
  {
    //std::tuple<> empty;
    //return handler(empty);
    return handler();
  });
}

}