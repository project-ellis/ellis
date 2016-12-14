#include <ellis/core/stream_decoder.hpp>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const decoding_status &s)
{
  const char *val;
  switch (s) {
    case decoding_status::CONTINUE:
      val = "CONTINUE";
      break;
    case decoding_status::END:
      val = "END";
      break;
    case decoding_status::ERROR:
      val = "ERROR";
      break;
  }
  os << val;

  return os;
}


} /*  namespace ellis */
