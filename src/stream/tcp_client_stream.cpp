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


#include <ellis/stream/tcp_client_stream.hpp>

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


tcp_stream::tcp_stream(UNUSED const char *host, UNUSED const char *port) {
  // int fd = tcpclientopen(host, port);  // if only that easy
  // TODO: throw exception on tcpclientopen
  // m_fdis = make_unique<fd_input_stream>(fd));
  // m_fdos = make_unique<fd_output_stream>(fd));
}

tcp_stream::~tcp_stream() {
  // close(m_fd);
}

bool tcp_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  return m_fdis->next_input_buf(buf, bytecount);
}

void tcp_stream::put_back(size_t bytecount) {
  return m_fdis->put_back(bytecount);
}

unique_ptr<err> tcp_stream::extract_input_error() {
  return m_fdis->extract_input_error();
}

bool tcp_stream::next_output_buf(byte **buf, size_t *bytecount) {
  return m_fdos->next_output_buf(buf, bytecount);
}

bool tcp_stream::emit(size_t bytecount) {
  return m_fdos->emit(bytecount);
}

unique_ptr<err> tcp_stream::extract_output_error() {
  return m_fdos->extract_output_error();
}


}  /* namespace ellis */
