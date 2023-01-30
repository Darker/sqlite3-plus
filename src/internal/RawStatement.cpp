#include "internal/RawStatement.h"
#include "../private/Database_Private.h"
#include "exceptions/SQLiteError.h"
#include "generic/Finally.h"

#include <limits>
#include <iostream>

#include "sqlite3.h"

namespace sqlitepp
{

struct RawStatement::Private
{
  sqlite3_stmt* statement = nullptr;
};

RawStatement::RawStatement(const std::string& query, Database* db, PrepareFlags flags)
  : _private(new Private)
  , _query(query)
  , _db(db)
  , _flags(flags)
  , _binder(*this)
{}

void RawStatement::Execute(const RowCallback& rowHandler)
{
  _binder.Reset();
  _executing = true;
  Finally f{ [this]()
    {
      _executing = false;
    }
  };

  bool done = true;
  do
  {
    auto result = sqlite3_step(_private->statement);
    done = true;
    switch (result)
    {
      case SQLITE_ROW: 
      {
        ReadHelper helper(*this);
        done = !rowHandler(helper);
        break;
      }
      case SQLITE_BUSY: 
      {
        done = false;
        break; 
      }
      case SQLITE_DONE:
      {
        break;
      }
      default:
      {
        std::cout << sqlite3_errmsg(_db->_private->db) << "\n";
        throw SQLiteCodedError("Failed to perform step on a prepared statement", (ResultCode)result);
      }
    }

  } while (!done);
}

void RawStatement::Init()
{
  if (!_initCalled)
  {
    _initCalled = true;
    const char* tail = nullptr;
    if (_query.length() >= (std::size_t)std::numeric_limits<int>::max())
    {
      throw SQLiteError(std::format("Cannot prepare query of size {}, it exceeds int size", _query.length()));
    }
    int result = sqlite3_prepare_v3(_db->_private->db, _query.data(), (int)_query.length(), static_cast<unsigned int>(_flags), &_private->statement, &tail);

    if (result != SQLITE_OK)
    {
      throw SQLiteCodedError("Error preparing statement.", static_cast<ResultCode>(result));
    }

    // remember the bind parameter count for sanity checks
    _bindCount = sqlite3_bind_parameter_count(_private->statement);
  }
}

//void RawStatement::Execute()
//{
//
//}

void RawStatement::BindHelper::Bind(int intval)
{
  bindSanityCheck();
  sqlite3_bind_int(_stmt._private->statement, ++index, intval);
}

void RawStatement::BindHelper::Bind(__int64 intval)
{
  bindSanityCheck();
  sqlite3_bind_int64(_stmt._private->statement, ++index, intval);
}

void RawStatement::BindHelper::Bind(double doubleVal)
{
  bindSanityCheck();
  int result = sqlite3_bind_double(_stmt._private->statement, ++index, doubleVal);
  if (result != SQLITE_OK)
  {
    std::cout << sqlite3_errmsg(_stmt._db->_private->db) << "\n";
    throw SQLiteCodedError("Failed to bind", (ResultCode)result);
  }
}

void RawStatement::BindHelper::Bind(const char* strval)
{
  bindSanityCheck();
  int result = sqlite3_bind_text(_stmt._private->statement, ++index, strval, -1, SQLITE_STATIC);

  if (result != SQLITE_OK)
  {
    std::cout << sqlite3_errmsg(_stmt._db->_private->db) << "\n";
    throw SQLiteCodedError("Failed to bind", (ResultCode)result);
  }
}

void RawStatement::BindHelper::Bind(const char* strval, std::size_t strLen)
{
  bindSanityCheck();
  int result = SQLITE_ERROR;

  if (strLen >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
  {
    result = sqlite3_bind_text64(_stmt._private->statement, ++index, strval, static_cast<sqlite_uint64>(strLen), SQLITE_STATIC, SQLITE_UTF8);
  }
  else
  {
    result = sqlite3_bind_text(_stmt._private->statement, ++index, strval, static_cast<int>(strLen), SQLITE_STATIC);
  }

  if (result != SQLITE_OK)
  {
    std::cout << sqlite3_errmsg(_stmt._db->_private->db) << "\n";
    throw SQLiteCodedError("Failed to bind", (ResultCode)result);
  }
}

void RawStatement::BindHelper::Bind(const void* blobData, std::size_t dataLen)
{
  bindSanityCheck();
  if (dataLen >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
  {
    sqlite3_bind_blob64(_stmt._private->statement, ++index, blobData, static_cast<sqlite_uint64>(dataLen), SQLITE_STATIC);
  }
  else
  {
    sqlite3_bind_blob(_stmt._private->statement, ++index, blobData, static_cast<int>(dataLen), SQLITE_STATIC);
  }
}

RawStatement::BindHelper::~BindHelper()
{
  //if (index > 0)
  //{
  //  
  //}
}

int RawStatement::ReadHelper::ReadInt()
{
  readSanityCheck();
  int current = index;
  auto result = sqlite3_column_int(_stmt._private->statement, index++);
  std::cout << std::format("Read int from column {} -> {}\n", current, result);
  return result;
}

__int64 RawStatement::ReadHelper::ReadLongInt()
{
  readSanityCheck();
  return sqlite3_column_int64(_stmt._private->statement, index++);
}

double RawStatement::ReadHelper::ReadDouble()
{
  readSanityCheck();
  int current = index;
  auto result = sqlite3_column_double(_stmt._private->statement, index++);
  std::cout << std::format("Read double from column {} -> {}\n", current, result);
  return result;
}

StrUnowned RawStatement::ReadHelper::ReadString()
{
  readSanityCheck();
  ++index;
  return { 
    (StrUnowned::byte_t*)sqlite3_column_text(_stmt._private->statement, index - 1),
    (std::size_t)sqlite3_column_bytes(_stmt._private->statement, index - 1)
  };
}

BytesUnowned RawStatement::ReadHelper::ReadBlob()
{
  readSanityCheck();
  ++index;
  return {
    (BytesUnowned::byte_t*)sqlite3_column_text(_stmt._private->statement, index - 1),
    (std::size_t)sqlite3_column_bytes(_stmt._private->statement, index - 1)
  };
}

void RawStatement::ReadHelper::readSanityCheck() const
{
  int count = sqlite3_column_count(_stmt._private->statement);
  if (count <= index)
    throw SQLiteError(std::format("Cannot read column #{}, only {} columns available", index, count));
}

RawStatement::~RawStatement()
{

}

}
