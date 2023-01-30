#pragma once
#include "Database.h"

struct sqlite3;

namespace sqlitepp
{

struct Database::Private
{
  sqlite3* db = nullptr;
};

}