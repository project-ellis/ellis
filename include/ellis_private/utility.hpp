/*
 * @file ellis_private/utility.hpp
 *
 * @brief utility code private to Ellis.
 *
 */

// TODO: doc
template <typename T, typename U>
static inline U union_cast(T x)
{
  union {
    T t;
    U u;
  } val;

  val.t = x;
  return val.u;
}
