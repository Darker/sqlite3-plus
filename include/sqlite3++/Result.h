#pragma once
#include "ResultRow.h"

namespace sqlitepp
{

template<typename ...TColValue>
class Result
{
public:
  ResultRow<TColValue> NextRow()
  {

  }
private:

};

}