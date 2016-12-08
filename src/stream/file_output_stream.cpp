#include <ellis/stream/file_output_stream.hpp>

#include <ellis/private/err.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


file_output_stream::file_output_stream(const char *) {
  // int fd = open(filename, "w");
  // TODO: throw exception on bad filename
  // m_fdos.reset(new fd_output_stream(fd));
}

file_output_stream::~file_output_stream() {
  // m_fdos.reset();
  // close(m_fd);
}

bool file_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  return m_fdos->next_output_buf(buf, bytecount);
}

bool file_output_stream::emit(size_t bytecount) {
  return m_fdos->emit(bytecount);
}

unique_ptr<err> file_output_stream::get_output_error() {
  return m_fdos->get_output_error();
}


}  /* namespace ellis */
