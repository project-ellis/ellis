#include <ellis/core/stream_decoder.hpp>

#include <ellis/core/system.hpp>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const decoding_status &s)
{
  switch (s) {
    case decoding_status::MAY_CONTINUE:
      return os << "MAY_CONTINUE";
    case decoding_status::MUST_CONTINUE:
      return os << "MUST_CONTINUE";
    case decoding_status::END:
      return os << "END";
    case decoding_status::ERROR:
      return os << "ERROR";
  }
  ELLIS_ASSERT_UNREACHABLE();
}


} /*  namespace ellis */
