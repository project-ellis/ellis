#include <ellis/stream/file_input_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace ellis {


file_input_stream::file_input_stream(const char *filename) {
  if (strcmp(filename, "-") == 0) {
    m_fd = 0;
  } else {
    m_fd = open(filename, O_RDONLY);
  }
  if (m_fd < 0) {
    // TODO: map errno for more specifics
    THROW_ELLIS_ERR(IO, "bad pathname: " << filename);
  }
  //m_fdis.reset(new fd_input_stream(m_fd));
}

file_input_stream::~file_input_stream() {
  m_fdis.reset();
  if (m_fd > 0) {
    close(m_fd);
    m_fd = -1;
  }
}

bool file_input_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  return m_fdis->next_input_buf(buf, bytecount);
}

void file_input_stream::put_back(size_t bytecount) {
  return m_fdis->put_back(bytecount);
}

unique_ptr<err> file_input_stream::extract_input_error() {
  return m_fdis->extract_input_error();
}


}  /* namespace ellis */
