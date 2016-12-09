#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <ellis/array_node.hpp>
#include <ellis/binary_node.hpp>
#include <ellis/codec/delimited_text.hpp>
#include <ellis/map_node.hpp>
#include <ellis/private/err.hpp>
#include <ellis/private/using.hpp>
#include <ellis/stream/cpp_input_stream.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <iostream>
#include <sstream>
#include <string.h>

namespace ellis {


/** Synchronous (blocking) load of an ellis node from the given input stream,
 * using the given decoder.
 *
 * On success, returns the newly constructed node.
 *
 * On failure, returns nullptr, and sets err_ret.
 */
unique_ptr<node> load(
    sync_input_stream *in,
    stream_decoder *deco,
    unique_ptr<err> *err_ret)
{
  deco->reset();
  while (1) {
    byte *buf = nullptr;
    size_t buf_remain = 0;
    /* Need another block; request it. */
    if (! in->next_input_buf(&buf, &buf_remain)) {
      *err_ret = in->get_input_error();
      return nullptr;
    }
    assert(buf != nullptr);
    assert(buf_remain > 0);
    /* Give block to decoder. */
    auto st = deco->consume_buffer(buf, &buf_remain);
    if (st != decoding_status::CONTINUE) {
      /* buf_remain has been updated to reflect unconsumed bytes remaining. */
      in->put_back(buf_remain);
      if (st == decoding_status::END) {
        return deco->get_node();
        //return std::move(deco->get_node());
      } else {
        *err_ret = deco->get_error();
        return nullptr;
      }
    }
    else {
      assert(buf_remain == 0);
      /* We consumed the whole buffer, not done yet; continue the loop. */
    }
  }
  assert(0);  /* should be unreachable */
  return nullptr;
}


template<typename TSTREAM, typename TDECODER>
unique_ptr<node> load(TSTREAM &&in, TDECODER &&deco)
{
  unique_ptr<err> e;
  unique_ptr<node> rv = load(&in, &deco, &e);
  if (e) {
    throw *e;
  }
  return rv;
}


/** Synchronous (blocking) dump of an ellis node to the given output stream,
 * using the given encoder.
 *
 * On success, true.
 *
 * On failure, returns false, and sets err_ret.
 */
bool dump(
    const node *nod,
    sync_output_stream *out,
    stream_encoder *enco,
    unique_ptr<err> *err_ret)
{
  enco->reset(nod);
  while (1) {
    byte *buf = nullptr;
    size_t bytecount = 0;
    /* Request a buffer to fill data into. */
    if (! out->next_output_buf(&buf, &bytecount)) {
      *err_ret = std::move(out->get_output_error());
      return false;
    }
    /* Have encoder fill the buffer. */
    auto st = enco->fill_buffer(buf, &bytecount);
    /* Emit whatever we were given to emit, regardless of error status. */
    if (! out->emit(bytecount)) {
      *err_ret = std::move(out->get_output_error());
      return false;
    }
    if (st == encoding_status::END) {
      return true;
    }
    else if (st == encoding_status::ERROR) {
      *err_ret = std::move(enco->get_error());
    }
    else if (st == encoding_status::CONTINUE) {
      continue;
    }
    else {
      assert(0);
    }
  }
  assert(0);  /* should be unreachable */
  return nullptr;
}


template<typename TSTREAM, typename TENCODER>
void dump(const node *nod, TSTREAM &&out, TENCODER &&enco)
{
  unique_ptr<err> e;
  dump(nod, &out, &enco, &e);
  if (e) {
    throw *e;
  }
}


}  /* namespace ellis */


int main() {
  using namespace ellis;
  //auto n = load(mem_input_stream("[1,2,3]"), json_decoder());
  //dump(n, file_output_stream("/tmp/lame.json"), json_encoder());
  //file_input_stream is("/tmp/lame.json");
  //json_decoder jd;
  //unique_ptr<node> n1(load(is, jd));
  //unique_ptr<node> n2(load(is, jd));
  std::stringstream ss1;
  ss1 << "one" << std::endl << "two" << std::endl << "three" << std::endl;
  auto s1 = ss1.str();
  auto n = load(cpp_input_stream(ss1), delimited_text_decoder());
  std::stringstream ss2;
  dump(n.get(), cpp_output_stream(ss2), delimited_text_encoder());
  auto s2 = ss2.str();
  assert(s1 == s2);
  return 0;
}
