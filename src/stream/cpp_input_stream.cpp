#include <ellis/stream/cpp_input_stream.hpp>

#include <ellis/private/err.hpp>
#include <ellis/private/using.hpp>

namespace ellis {

cpp_input_stream::cpp_input_stream(std::istream &is) : m_is(is) {
}

bool cpp_input_stream::next_input_buf(byte **buf, int *bytecount) {
  if (m_pos < m_avail) {
    /* We have some leftover buffer from earlier.  Return that. */
    *buf = m_buf + m_pos;
    *bytecount = m_avail - m_pos;
    return true;
  }
  /* No more bytes in current block?  Then try to get another one. */
  if (! m_is) {
    // TODO: check fail bits?
    // TODO: not PARSING_ERROR
    m_err.reset(new MAKE_ELLIS_ERR(err_code::PARSING_ERROR, "end of file"));
    return false;
  }
  m_pos = 0;
  m_is.read((char*)m_buf, sizeof(buf));
  m_avail = (int)m_is.gcount();
  if (m_avail <= 0) {
    // TODO: check fail bits?
    // TODO: not PARSING_ERROR
    m_err.reset(new MAKE_ELLIS_ERR(err_code::PARSING_ERROR, "end of file"));
    return false;
  }
  return true;
}

void cpp_input_stream::put_back(int bytecount) {
  m_pos = m_avail - bytecount;
}

unique_ptr<err> cpp_input_stream::get_input_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
