#include <ellis/stream/mem_input_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {

mem_input_stream::mem_input_stream(const void *buf, size_t len) :
  m_buf((const byte *)buf), m_len(len)
{
}

bool mem_input_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  if (m_pos < m_len) {
    /* We have some leftover buffer from earlier.  Return that. */
    *buf = m_buf + m_pos;
    *bytecount = m_len - m_pos;
    /* Treat input as consumed unless put_back is called. */
    m_pos = m_len;
    return true;
  } else {
    m_err.reset(new MAKE_ELLIS_ERR(IO, "end of stream reached"));
    return false;
  }
}

void mem_input_stream::put_back(size_t bytecount) {
  m_pos = m_len - bytecount;
}

unique_ptr<err> mem_input_stream::extract_input_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
