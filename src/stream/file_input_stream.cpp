#include <ellis/stream/file_input_stream.hpp>

#include <ellis/private/core/err.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


file_input_stream::file_input_stream(const char *) {
  // int fd = open(filename, "r");
  // TODO: throw exception on bad filename
  // m_fdis.reset(new fd_input_stream(fd));
}

file_input_stream::~file_input_stream() {
  // m_fdis.reset();
  // close(m_fd);
}

bool file_input_stream::next_input_buf(byte **buf, size_t *bytecount) {
  return m_fdis->next_input_buf(buf, bytecount);
}

void file_input_stream::put_back(size_t bytecount) {
  return m_fdis->put_back(bytecount);
}

unique_ptr<err> file_input_stream::extract_input_error() {
  return m_fdis->extract_input_error();
}


}  /* namespace ellis */
