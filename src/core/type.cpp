#include <ellis/core/type.hpp>

#include <ellis/private/using.hpp>


namespace ellis {


const char *type_str(type t)
{
  switch (t) {
    case type::NIL:
      return "NIL";

    case type::BOOL:
      return "BOOL";

    case type::INT64:
      return "INT64";

    case type::DOUBLE:
      return "DOUBLE";

    case type::U8STR:
      return "U8STR";

    case type::ARRAY:
      return "ARRAY";

    case type::BINARY:
      return "BINARY";

    case type::MAP:
      return "MAP";
  }
  /* Never reached. */
  assert(0);
}


}  /* namespace ellis */
