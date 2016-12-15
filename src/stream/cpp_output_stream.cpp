#include <ellis/stream/cpp_output_stream.hpp>

#include <ellis_private/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


cpp_output_stream::cpp_output_stream(std::ostream &os) : m_os(os) {
}

bool cpp_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  *buf = m_buf;
  *bytecount = sizeof(m_buf);
  return true;
}

bool cpp_output_stream::emit(size_t bytecount) {
  // TODO: handle failure
  m_os.write((char*)m_buf, bytecount);
  return true;
}

unique_ptr<err> cpp_output_stream::extract_output_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
