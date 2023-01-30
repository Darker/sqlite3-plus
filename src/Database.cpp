#include "Database.h"
#include "exceptions/SQLiteError.h"
#include "ResultCode.h"
#include "private/Database_Private.h"

#include "sqlite3.h"

#include <string>

namespace sqlitepp
{

Database::Database() : _private(new Private)
{}

void Database::open(const char* path, OpenFlags flags)
{
  int result = sqlite3_open_v2(path, &_private->db, static_cast<int>(flags), nullptr);
  if (result != SQLITE_OK)
  {
    throw SQLiteError(sqlite3_errmsg(_private->db));
  }
}

void Database::exec(const char* statement, bool wait)
{
  int result;
  char* errorMessage = nullptr;
  while ((result = sqlite3_exec(_private->db, statement, nullptr, nullptr, &errorMessage)) == SQLITE_BUSY && wait);

  if (result != SQLITE_OK)
  {
    std::string errorMsg(errorMessage == nullptr ? "" : errorMessage);
    sqlite3_free(errorMessage);
    throw SQLiteCodedError(errorMsg, static_cast<ResultCode>(result));
  }
}

bool Database::isOpen()
{
  return _private->db != nullptr;
}

Database::~Database()
{
  sqlite3_close_v2(_private->db);
  _private->db = nullptr;
}

}


