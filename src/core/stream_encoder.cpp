#include <ellis/core/stream_encoder.hpp>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const encoding_status &s)
{
  const char *val;
  switch (s) {
    case encoding_status::CONTINUE:
      val = "CONTINUE";
      break;
    case encoding_status::END:
      val = "END";
      break;
    case encoding_status::ERROR:
      val = "ERROR";
      break;
  }
  os << val;

  return os;
}


} /*  namespace ellis */
