#include "traits/BindTraits.h"
#include "internal/RawStatement.h"

namespace sqlitepp
{


class Binder
{
public:
  virtual int sqlite_bind(RawStatement& statement) = 0;
};


//class BindValue::Private
//{
//public:
//
//};


}