#pragma once
#include <concepts>

namespace sqlitepp
{

template<typename TFunc> requires std::invocable<TFunc>
struct Finally
{
  TFunc functor;

  ~Finally() { functor(); }
};

}