#pragma once
namespace sqlitepp
{

template<unsigned int TIndex, typename ...TColValue>
struct get_nth_from_variadric_type;

template<unsigned int TIndex, typename Head, typename... Tail >
struct get_nth_from_variadric_type<TIndex, Head, Tail...>
  : get_nth_from_variadric_type<TIndex - 1, Tail...>
{};

template<typename Head, typename... Tail>
struct get_nth_from_variadric_type<0, Head, Tail...>
{
  using type = Head;
};

// Retrieves nth type from a variadric value list
template<unsigned int TIndex, typename ...TColValue>
using get_nth_from_variadric = typename get_nth_from_variadric_type<TIndex, TColValue...>::type;


}