#include <ellis/core/stream_decoder.hpp>

#include <ellis/core/system.hpp>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const decoding_status &s)
{
  switch (s) {
    case decoding_status::CONTINUE:
      return os << "CONTINUE";
    case decoding_status::END:
      return os << "END";
    case decoding_status::ERROR:
      return os << "ERROR";
  }
  ELLIS_ASSERT_UNREACHABLE();
}


} /*  namespace ellis */
