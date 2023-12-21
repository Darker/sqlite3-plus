// BasicSQLite.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sqlite3++/Database.h>
#include <sqlite3++/Statement.h>
#include <sqlite3++/logging/OstreamLogger.h>

int ma_in()
{
  sqlitepp::OstreamLoggerBorrowed logger(std::cout);

  sqlitepp::Database db;
  db.open("test.sqlite");
  db.exec("PRAGMA user_version = 0");
  db.exec(R"SQL(
        -- Create table for the data
        CREATE TABLE IF NOT EXISTS "test" (
	        "key"	TEXT NOT NULL UNIQUE,
	        "created" DATETIME,
          "updated" DATETIME,
          "quantity" INT NOT NULL DEFAULT 0,
          "quantity_type" SMALLINT DEFAULT 0,

          --"lastread" INTEGER NOT NULL DEFAULT 0,
	        --PRIMARY KEY("key")
        );
        )SQL");
  sqlitepp::Statement<> teststmt(R"SQL(
        INSERT INTO test (key, quantity)
        VALUES("ddd", 0)
        )SQL");
  teststmt.Init(&db);
  teststmt.execute(
    []()
    {
      return true;
    }
  );

  double readDouble;
  int readInt;
  std::string readStr;
  sqlitepp::Statement<double, int, sqlitepp::StrUnowned> readstmt(R"SQL(
        SELECT data, int_num, key FROM test WHERE key = ?
        )SQL");

  readstmt.Init(&db);

  readstmt.execute(
    [&readDouble, &readInt, &readStr](double rdbl, int rint, sqlitepp::StrUnowned rstr)
    {
      readDouble = rdbl;
      readInt = rint;
      readStr = rstr.data();
      //readDouble = std::get<0>(tpl);
      //readInt = std::get<1>(tpl);
      //readStr = std::get<2>(tpl).data();
      return false;
    },
    "test_name"
  );
  logger.info("Read values: double={}, int={} for key: {}", readDouble, readInt, readStr);

  return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
