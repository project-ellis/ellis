#include <ellis/core/immigration.hpp>

#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>


namespace ellis {


unique_ptr<node> load(
    sync_input_stream *in,
    stream_decoder *deco,
    unique_ptr<err> *err_ret)
{
  ELLIS_ASSERT(err_ret != nullptr);
  deco->reset();
  unique_ptr<node> rv;
  while (1) {
    byte *buf = nullptr;
    size_t buf_remain = 0;
    /* Need another block; request it. */
    if (! in->next_input_buf(&buf, &buf_remain)) {
      *err_ret = in->extract_input_error();
      goto error_return;
    }
    ELLIS_ASSERT(buf != nullptr);
    ELLIS_ASSERT(buf_remain > 0);
    /* Give block to decoder. */
    auto st = deco->consume_buffer(buf, &buf_remain);
    if (st != decoding_status::CONTINUE) {
      /* buf_remain has been updated to reflect unconsumed bytes remaining. */
      in->put_back(buf_remain);
      if (st == decoding_status::END) {
        rv = deco->extract_node();
        goto success_return;
      } else {
        *err_ret = deco->extract_error();
        goto error_return;
      }
    }
    else {
      ELLIS_ASSERT_EQ(buf_remain, 0);
      /* We consumed the whole buffer, not done yet; continue the loop. */
    }
  }
  ELLIS_ASSERT_UNREACHABLE();

success_return:
  ELLIS_ASSERT(rv);
  return rv;

error_return:
  ELLIS_ASSERT(*err_ret);
  return nullptr;
}


}  /* namespace ellis */
