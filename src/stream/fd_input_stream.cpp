#include <ellis/stream/fd_input_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>
#include <unistd.h>

namespace ellis {

fd_input_stream::fd_input_stream(int fd) : m_fd(fd) {
}

bool fd_input_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  int n = 0;
  if (m_pos < m_avail) {
    /* We have some leftover buffer from earlier.  Return that. */
    goto give_buffer;
  }

  /* No more bytes in current block?  Then try to get another one. */
  m_pos = 0;
  m_avail = 0;
  while (1) {
    n = read(m_fd, m_buf, sizeof(m_buf));
    if (n < 0 && errno == EINTR) {
      continue;
    }
    else {
      break;
    }
  }
  if (n < 0) {
    /* TODO: extract details via strerror?  Use errno? */
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "I/O error");
    return false;
  }
  else if (n == 0) {
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "end of file");
    return false;
  }
  /* Got some data. */
  m_avail = n;

give_buffer:
  *buf = (const byte *)(m_buf + m_pos);
  *bytecount = m_avail - m_pos;
  /* Treat input as consumed unless put_back is called. */
  m_pos = m_avail;
  return true;
}

void fd_input_stream::put_back(size_t bytecount) {
  m_pos = m_avail - bytecount;
}

unique_ptr<err> fd_input_stream::extract_input_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
