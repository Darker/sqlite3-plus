#pragma once
#include "Logger.h"

namespace sqlitepp
{

std::string&& buildModuleName(Logger* parent, std::string_view current);

class ModuleLogger : public Logger
{
public:
  ModuleLogger(std::string_view moduleName, Logger* parent)
    : moduleName(buildModuleName(parent, moduleName))
    , parent(parent)
  {}

  virtual void write(Level level, std::string_view moduleName, std::string_view message) override { parent->write(level, moduleName, message); }
  virtual const char* getModuleName() const override { return moduleName.data(); }
private:
  std::string moduleName;
  Logger* parent;
};


static inline std::string&& buildModuleName(Logger* parent, std::string_view current)
{
  std::string_view parentName{ parent->getModuleName() };
  if (parentName.size() == 0)
  {
    return std::string{ current };
  }
  else
  {
    std::string res;
    res.reserve(parentName.size() + current.size() + 2);
    res.append(parentName);
    res.append(".");
    res.append(current);
    return std::move(res);
  }
}

}
