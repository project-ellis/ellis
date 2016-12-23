#include <ellis/stream/file_output_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace ellis {


file_output_stream::file_output_stream(const char *filename) {
  m_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (m_fd < 0) {
    throw MAKE_ELLIS_ERR(TODO, filename);
  }
  //m_fdos.reset(new fd_output_stream(m_fd));
}

file_output_stream::~file_output_stream() {
  m_fdos.reset();
  if (m_fd >= 0) {
    close(m_fd);
    m_fd = 0;
  }
}

bool file_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  return m_fdos->next_output_buf(buf, bytecount);
}

bool file_output_stream::emit(size_t bytecount) {
  return m_fdos->emit(bytecount);
}

unique_ptr<err> file_output_stream::extract_output_error() {
  return m_fdos->extract_output_error();
}


}  /* namespace ellis */
