#pragma once
#include "Logger.h"

namespace sqlitepp
{

class DummyLogger : public Logger
{
public:
  virtual void write(Level level, std::string_view moduleName, std::string_view message) override {}
};

}
