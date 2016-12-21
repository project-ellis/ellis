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
  auto st = decoding_status::CONTINUE;
  while (st == decoding_status::CONTINUE) {
    byte *buf = nullptr;
    size_t buf_remain = 0;
    /* Need another block; request it. */
    if (! in->next_input_buf(&buf, &buf_remain)) {
      /* No block available. */
      st = deco->terminate_stream();
    }
    else {
      /* Block obtained. */
      ELLIS_ASSERT(buf != nullptr);
      ELLIS_ASSERT(buf_remain > 0);
      /* Give block to decoder. */
      st = deco->consume_buffer(buf, &buf_remain);
    }
    switch (st) {
      case decoding_status::ERROR:
        in->put_back(buf_remain);
        *err_ret = deco->extract_error();
        ELLIS_ASSERT(*err_ret);
        return nullptr;

      case decoding_status::END:
        in->put_back(buf_remain);
        return deco->extract_node();

      case decoding_status::CONTINUE:
        /* All the input should have been used; we're going to get more. */
        ELLIS_ASSERT_EQ(buf_remain, 0);
        break;
    }
  }
  ELLIS_ASSERT_UNREACHABLE();
}


}  /* namespace ellis */
