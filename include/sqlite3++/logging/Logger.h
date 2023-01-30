#pragma once
#include "../generic/NoCopy.h"
#include "../generic/std_format_polyfill.h"

#include <string>
#include <string_view>

namespace sqlitepp
{

class Logger
{
public:
  enum class Level
  {
    TRACE,
    INFO,
    WARN,
    ERROR,
    FATAL
  };

  static constexpr const char* LevelStr[] =
  {
    "TRACE",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
  };

  virtual void write(Level level, std::string_view moduleName, std::string_view message) = 0;

  static constexpr const char* EMPTY_STR = "";
  // Provides a name of a module (ie. library, class, function)
  // to allow filtering logs depending on where they come from
  virtual const char* getModuleName() const { return EMPTY_STR; }

protected:
  template<typename... TArgs>
  void format_write(Level level, const char* format, TArgs&&... args);

public:
  template<typename... TArgs>
  void trace(const char* format, TArgs&&... args) { format_write(Level::TRACE, format, args...); }

  template<typename... TArgs>
  void info(const char* format, TArgs&&... args) { format_write(Level::INFO, format, args...); }

  template<typename... TArgs>
  void warn(const char* format, TArgs&&... args) { format_write(Level::WARN, format, args...); }

  template<typename... TArgs>
  void error(const char* format, TArgs&&... args) { format_write(Level::ERROR, format, args...); }

  template<typename... TArgs>
  void fatal(const char* format, TArgs&&... args) { format_write(Level::FATAL, format, args...); }
};

template<typename ...TArgs>
inline void Logger::format_write(Level level, const char* format, TArgs&& ...args)
{
  std::string msg = std::vformat(format, std::make_format_args(args...));
  write(level, getModuleName(), msg);
}

}
