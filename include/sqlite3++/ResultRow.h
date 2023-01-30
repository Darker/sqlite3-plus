#pragma once
#include "generic/NoCopy.h"
#include "generic/templates.h"

namespace sqlitepp
{

template<typename ...TColValue>
class ResultRow : public NoCopy
{
public:
  template<unsigned int TIndex>
  get_nth_from_variadric<TIndex, TColValue> GetValue() const
  {
    return get_nth_from_variadric<TIndex, TColValue>();
  }
  

};

}
