#pragma once
namespace sqlitepp
{

// https://stackoverflow.com/a/10766422/607407
// implementation details, users never invoke these directly
namespace detail
{
template <typename F, typename Tuple, bool Done, int Total, int... N>
struct call_impl
{
  static auto call(F f, Tuple&& t)
  {
    return call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
  }
};

template <typename F, typename Tuple, int Total, int... N>
struct call_impl<F, Tuple, true, Total, N...>
{
  static auto call(F f, Tuple&& t)
  {
    return f(std::get<N>(std::forward<Tuple>(t))...);
  }
};
} // namespace detail

template <typename F, typename Tuple>
auto call_with_tuple(F f, Tuple&& t)
{
  typedef typename std::decay<Tuple>::type ttype;
  return detail::call_impl<F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}

}
