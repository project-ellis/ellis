#include <ellis/codec/delimited_text.hpp>

#include <ellis/core/array_node.hpp>
#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


void delimited_text_decoder::_clear_ss() {
  /* Clear the string stream. */
  m_ss.str("");
  /* Reset any stream state such as errors. */
  m_ss.clear();
}

delimited_text_decoder::delimited_text_decoder() :
  m_node(new node(type::ARRAY))
{
}

node_progress delimited_text_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  const byte *p_end = buf + *bytecount;
  for (const byte *p = buf; p < p_end; p++) {
    // TODO: handle \r\n ?
    if (*p == '\n') {
      m_node->as_mutable_array().append(node(m_ss.str()));
      _clear_ss();
    } else {
      m_ss << *p;
    }
  }
  *bytecount = 0;
  if (m_ss.tellp() == 0) {
    return node_progress(std::move(m_node));
  }
  return node_progress(stream_state::CONTINUE);
}

node_progress delimited_text_decoder::chop()
{
  return node_progress(std::move(m_node));
}

void delimited_text_decoder::reset()
{
  m_node.reset(new node(type::ARRAY));
  _clear_ss();
}


void delimited_text_encoder::_clear_ss() {
  /* Clear the string stream. */
  m_ss.str("");
  /* Reset any stream state such as errors. */
  m_ss.clear();
}

delimited_text_encoder::delimited_text_encoder()
{
}

delimited_text_encoder::~delimited_text_encoder()
{
}

progress delimited_text_encoder::fill_buffer(
    byte *buf,
    size_t *bytecount)
{
  size_t ss_avail = m_ssend - m_sspos;
  size_t actual_bc = std::min(*bytecount, ss_avail);
  m_ss.read((char *)buf, actual_bc);
  m_sspos += actual_bc;
  *bytecount = actual_bc;
  if (m_sspos == m_ssend) {
    return progress(true);
  }
  return progress(stream_state::CONTINUE);
}

void delimited_text_encoder::reset(const node *new_node)
{
  _clear_ss();
  for (size_t i = 0; i < new_node->as_array().length(); i++) {
    m_ss << new_node->as_array()[i].as_u8str() << "\n";
  }
  m_ss.flush();
  m_sspos = 0;
  m_ssend = m_ss.tellp();
}


}  /* namespace ellis */
