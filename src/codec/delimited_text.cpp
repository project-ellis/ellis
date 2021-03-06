/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <ellis/codec/delimited_text.hpp>

#include <ellis/core/array_node.hpp>
#include <ellis/core/data_format.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/system.hpp>
#include <ellis/core/u8str_node.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


static class dummy_init {
public:
  dummy_init()
  {
    system_add_data_format(
        make_unique<const data_format>(
          "builtin.txtfile.lines",
          "txt",
          "Text file representing an array of lines",
          [](){ return unique_ptr<decoder>(
            new ellis::delimited_text_decoder()); },
          [](){ return unique_ptr<encoder>(
            new ellis::delimited_text_encoder()); }));
  }
} unused_dummy_init;


void delimited_text_decoder::_clear_ss() {
  /* Clear the string stream. */
  m_ss.str("");
  /* Reset any stream state such as errors. */
  m_ss.clear();
}

delimited_text_decoder::delimited_text_decoder() :
  m_node(make_unique<node>(type::ARRAY))
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
  m_node = make_unique<node>(type::ARRAY);
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
