#include <ellis/core/type.hpp>

#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>


namespace ellis {


/** We're going to do macro tricks here, while leaving the header clean.
 *
 * Thus the following list needs to match what's in the header, but we double
 * check to make sure it maches in two ways: we static_assert that the index
 * of the last item in this list is the same as the last item in the original
 * enum, and we also have a switch statement that would yield a compiler
 * warning if there were a mismatch.   The static_assert protects us in case
 * the compiler warnings should ever not work as intended. */
#define DATUM_TYPES \
  ENTRY(NIL) \
  ENTRY(BOOL) \
  ENTRY(INT64) \
  ENTRY(DOUBLE) \
  ENTRY(U8STR) \
  ENTRY(ARRAY) \
  ENTRY(BINARY) \
  ENTRY(MAP) \
  /* End of DATUM_TYPES */

/** A helper function to count the number of enums passed to it. */
template<type... T>
constexpr int count_args() { return sizeof...(T); }

#define ENTRY(X) type::X,
static_assert(count_args<DATUM_TYPES type::enum_max>()
    == (int)type::enum_max + 2,
    "DATUM_TYPES does not match type enum");
#undef ENTRY

const char *type_str(type x)
{
  switch (x) {
#define ENTRY(X) case type::X: return #X;
  DATUM_TYPES
#undef ENTRY
  }
  ELLIS_ASSERT_UNREACHABLE();
}


/* Same tricks as above for a different type. */
#define STREAM_STATES \
  ENTRY(CONTINUE) \
  ENTRY(SUCCESS) \
  ENTRY(ERROR) \
  /* End of STREAM_STATES */

/** A helper function to count the number of enums passed to it. */
template<stream_state... T>
constexpr int count_args() { return sizeof...(T); }

#define ENTRY(X) stream_state::X,
static_assert(count_args<STREAM_STATES stream_state::enum_max>()
    == (int)stream_state::enum_max + 2,
    "STREAM_STATES does not match stream_state enum");
#undef ENTRY

const char *enum_name(stream_state x)
{
  switch (x) {
#define ENTRY(X) case stream_state::X: return #X;
  STREAM_STATES
#undef ENTRY
  }
  ELLIS_ASSERT_UNREACHABLE();
}


std::ostream & operator<<(std::ostream & os, const stream_state &s)
{
  return os << enum_name(s);
}


}  /* namespace ellis */
