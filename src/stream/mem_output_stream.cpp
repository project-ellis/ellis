#include <ellis/stream/mem_output_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


mem_output_stream::mem_output_stream(void *buf, size_t len) :
  m_buf((byte *)buf), m_len(len)
{
  ELLIS_ASSERT_GT(m_len, 0);
}

bool mem_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  if (m_pos < m_len - 1) {
    /* We have some leftover buffer from earlier.  Return that. */
    *buf = m_buf + m_pos;
    *bytecount = m_len - 1 - m_pos;
    return true;
  } else {
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "end of memory stream reached");
    return false;
  }
}

bool mem_output_stream::emit(size_t bytecount) {
  m_pos += bytecount;
  ELLIS_ASSERT_LT(m_pos, m_len);
  m_buf[m_pos] = '\0';

  return true;
}

unique_ptr<err> mem_output_stream::extract_output_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
