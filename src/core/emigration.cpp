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


#include <ellis/core/emigration.hpp>

#include <ellis/core/data_format.hpp>
#include <ellis/core/system.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <ellis/stream/fd_output_stream.hpp>
#include <ellis/stream/file_output_stream.hpp>
#include <ellis/stream/mem_output_stream.hpp>
#include <ellis_private/convenience/file.hpp>
#include <ellis_private/using.hpp>


namespace ellis {


void dump(
    const node *nod,
    sync_output_stream *out,
    encoder *enco)
{
  enco->reset(nod);
  while (1) {
    byte *buf = nullptr;
    size_t bytecount = 0;
    /* Request a buffer to fill data into. */
    if (! out->next_output_buf(&buf, &bytecount)) {
      throw *(out->extract_output_error());
    }
    /* Have encoder fill the buffer. */
    auto st = enco->fill_buffer(buf, &bytecount);
    /* Emit whatever we were given to emit, regardless of error status. */
    if (! out->emit(bytecount)) {
      throw *(out->extract_output_error());
    }
    switch (st.state()) {
      case stream_state::SUCCESS:
        return;

      case stream_state::ERROR:
        throw *(st.extract_error());

      case stream_state::CONTINUE:
        break;  /* but continue loop */
    }
  }
  ELLIS_ASSERT_UNREACHABLE();
}


void dump_file(
    const node *nod,
    const char *filename,
    encoder *enco)
{
  dump(nod, file_output_stream(filename), *enco);
}


void dump_mem(
    const node *nod,
    void *buf,
    size_t len,
    encoder *enco)
{
  dump(nod, mem_output_stream(buf, len), *enco);
}


void dump_stream(
    const node *nod,
    std::ostream &os,
    encoder *enco)
{
  dump(nod, cpp_output_stream(os), *enco);
}


void dump_file_autoencode(
    const node *nod,
    const char *filename)
{
  string exten = get_extension(filename);
  auto fmts = system_lookup_data_formats_by_extension(exten.c_str());
  if (fmts.empty()) {
    THROW_ELLIS_ERR(NO_SUCH,
        "No valid format found for output extension (" << exten << ")");
  }

  bool write_success = false;
  string failmsg = "no encoders found";
  for (auto fmt: fmts) {
    auto enc = (fmt->m_make_encoder)();
    if (!enc) {
      /* No encoder function; move on to the next format. */
      continue;
    }
    try {
      dump_file(nod, filename, enc.get());
      write_success = true;
      break;
    }
    catch (const ellis::err &e) {
      /* Failed encode; remember error, but move on to the next format. */
      failmsg = e.msg();
    }
  }

  if (! write_success) {
    THROW_ELLIS_ERR(TRANSLATE_FAIL,
        "Unable to encode file (" << filename << "):" << failmsg);
  }
}


}  /* namespace ellis */
