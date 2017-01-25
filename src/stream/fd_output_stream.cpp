#include <ellis/stream/fd_output_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>
#include <unistd.h>

namespace ellis {


fd_output_stream::fd_output_stream(int fd) : m_fd(fd) {
}

bool fd_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  *buf = (byte *)m_buf;
  *bytecount = sizeof(m_buf);
  return true;
}

bool fd_output_stream::emit(size_t bytecount) {
  ELLIS_ASSERT_LTE(bytecount, sizeof(m_buf));
  size_t pos = 0;
  while (pos < bytecount) {
    int n = write(m_fd, m_buf, bytecount - pos);
    if (n == 0 || (n < 0 && errno == EINTR)) {
      continue;
    }
    else if (n < 0) {
      /* TODO: extract details via strerror?  Use errno? */
      m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "I/O error");
      return false;
    }
    /* We wrote some bytes. */
    pos += n;
  }
  return true;
}

unique_ptr<err> fd_output_stream::extract_output_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
