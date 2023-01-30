#pragma once
#include "flags.h"
#include "generic/NoCopy.h"

#include <memory>
#include <string_view>

namespace sqlitepp
{

class RawStatement;

class Database 
{
  friend class RawStatement;
public:
  enum class ExecResult
  {
    // Everything executed correctly
    OK,
    // The database is busy handling another request, try again later
    BUSY,
    // Statement execution failed
    ERROR,
  };

  Database();
  virtual ~Database();

  void open(const char* path, OpenFlags flags = OpenFlags::READWRITE | OpenFlags::CREATE);

  void exec(const char* statement, bool wait = true);

  bool isOpen();
protected:
  struct Private;
  std::unique_ptr<Private> _private;

private:



};


}