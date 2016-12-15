#include <ellis/core/stream_encoder.hpp>

#include <ellis/core/system.hpp>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const encoding_status &s)
{
  switch (s) {
    case encoding_status::CONTINUE:
      return os << "CONTINUE";
    case encoding_status::END:
      return os << "END";
    case encoding_status::ERROR:
      return os << "ERROR";
  }
  ELLIS_ASSERT_UNREACHABLE();
}


} /*  namespace ellis */
