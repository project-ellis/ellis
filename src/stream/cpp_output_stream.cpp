#include <ellis/stream/cpp_output_stream.hpp>

#include <ellis/private/err.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


cpp_output_stream::cpp_output_stream(std::ostream &os) : m_os(os) {
}

bool cpp_output_stream::next_output_buf(byte **buf, int *bytecount) {
  *buf = m_buf;
  *bytecount = sizeof(m_buf);
  return true;
}

bool cpp_output_stream::emit(int bytecount) {
  // TODO: handle failure
  m_os.write((char*)m_buf, bytecount);
  return true;
}

unique_ptr<err> cpp_output_stream::get_output_error() {
  return std::move(m_err);
}


}  /* namespace ellis */