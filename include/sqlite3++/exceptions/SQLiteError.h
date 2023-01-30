#pragma once
#include <stdexcept>
#include "../ResultCode.h"

namespace sqlitepp
{

class SQLiteError : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

class SQLiteCodedError : public SQLiteError
{
public:
  SQLiteCodedError(const std::string& errorMessage, ResultCode errorCode = ResultCode::ERROR)
    : SQLiteError(errorMessage)
    , _sqliteErrorCode(errorCode)
  {}

private:
  ResultCode _sqliteErrorCode;
};

}

