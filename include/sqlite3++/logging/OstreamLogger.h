#pragma once
#include "Logger.h"

#include <ostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <concepts>

namespace sqlitepp
{

//namespace detail
//{
//
//class NonOwnedOstream : public std::ostream
//{
//public:
//  explicit NonOwnedOstream(std::ostream& str) : std::ostream(str._)
//  {
//
//  }
//  template <class T>
//  NonOwnedOstream& operator<<(T&& x)
//  {
//    *str << std::forward<T>(x);
//    return *this;
//  }
//private:
//  std::ostream& str;
//}
//
//}

class OstreamLogger : public Logger
{
public:
  virtual void write(Level level, std::string_view moduleName, std::string_view message) override;
  virtual std::ostream& getStream() = 0;
};

void OstreamLogger::write(Level level, std::string_view moduleName, std::string_view message)
{
  using Clock = std::chrono::system_clock;

  auto now = Clock::now();
  auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto fraction = now - seconds;
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);

  auto t = Clock::to_time_t(now);

  const auto tm = std::gmtime(&t);
  getStream()
    << std::put_time(tm, "%FT%T") << '.' << milliseconds.count() << "Z "
    << Logger::LevelStr[(std::size_t)level] << " "
    << moduleName << " "
    << message
    << std::endl;
};

class OstreamLoggerBorrowed : public OstreamLogger
{
public:
  OstreamLoggerBorrowed(std::ostream& str) : borrowedStream(str) {}
  virtual std::ostream& getStream() { return borrowedStream; }
private:
  std::ostream& borrowedStream;
};

//template<std::derived_from<std::ostream> Tostream>
template<class Tostream> requires std::derived_from<Tostream, std::ostream>
class OstreamLoggerOwned : public OstreamLogger
{
public:
  OstreamLoggerOwned(Tostream&& stream) : stream(std::move(stream)) {}
  virtual std::ostream& getStream() { return stream; }
private:
  Tostream stream;
};

}
