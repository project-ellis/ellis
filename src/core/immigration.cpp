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
  auto st = decoding_status::MUST_CONTINUE;
  while (st == decoding_status::MAY_CONTINUE
      || st == decoding_status::MUST_CONTINUE) {
    byte *buf = nullptr;
    size_t buf_remain = 0;
    /* Need another block; request it. */
    if (! in->next_input_buf(&buf, &buf_remain)) {
      /* No block available. */
      goto forcibly_terminate_input_stream;
    }
    /* Block obtained. */
    ELLIS_ASSERT(buf != nullptr);
    ELLIS_ASSERT(buf_remain > 0);
    /* Give block to decoder. */
    st = deco->consume_buffer(buf, &buf_remain);
    switch (st) {
      case decoding_status::ERROR:
        in->put_back(buf_remain);
        *err_ret = deco->extract_error();
        goto error_return;

      case decoding_status::END:
        in->put_back(buf_remain);
        goto extract_return;

      case decoding_status::MAY_CONTINUE:
      case decoding_status::MUST_CONTINUE:
        /* All the input should have been used; we're going to get more. */
        ELLIS_ASSERT_EQ(buf_remain, 0);
        break;
    }
  }
  ELLIS_ASSERT_UNREACHABLE();

forcibly_terminate_input_stream:
  st = deco->terminate_stream();
  if (st == decoding_status::ERROR) {
    *err_ret = deco->extract_error();
    goto error_return;
  }

extract_return:
  return deco->extract_node();

error_return:
  ELLIS_ASSERT(*err_ret);
  return nullptr;
}


}  /* namespace ellis */
