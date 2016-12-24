#include <ellis/core/immigration.hpp>

#include <ellis/core/system.hpp>
#include <ellis/stream/cpp_input_stream.hpp>
#include <ellis/stream/fd_input_stream.hpp>
#include <ellis/stream/file_input_stream.hpp>
#include <ellis/stream/mem_input_stream.hpp>
#include <ellis_private/using.hpp>


namespace ellis {


unique_ptr<node> load(
    sync_input_stream *in,
    decoder *deco)
{
  deco->reset();
  unique_ptr<node> rv;
  auto st = node_progress(stream_state::CONTINUE);
  while (st.state() == stream_state::CONTINUE) {
    const byte *buf = nullptr;
    size_t buf_remain = 0;
    /* Need another block; request it. */
    if (! in->next_input_buf(&buf, &buf_remain)) {
      /* No block available. */
      st = deco->cleave();
    }
    else {
      /* Block obtained. */
      ELLIS_ASSERT(buf != nullptr);
      ELLIS_ASSERT(buf_remain > 0);
      /* Give block to decoder. */
      st = deco->consume_buffer(buf, &buf_remain);
    }
    switch (st.state()) {
      case stream_state::ERROR:
        in->put_back(buf_remain);
        throw *(st.extract_error());

      case stream_state::SUCCESS:
        in->put_back(buf_remain);
        return st.extract_value();

      case stream_state::CONTINUE:
        /* All the input should have been used; we're going to get more. */
        ELLIS_ASSERT_EQ(buf_remain, 0);
        break;
    }
  }
  ELLIS_ASSERT_UNREACHABLE();
}


//std::unique_ptr<node> load_fd(
//    int fd,
//    decoder *deco)
//{
//  return load(fd_input_stream(fd), *deco);
//}


std::unique_ptr<node> load_file(
    const char *filename,
    decoder *deco)
{
  return load(file_input_stream(filename), *deco);
}


std::unique_ptr<node> load_mem(
    const void *buf,
    size_t len,
    decoder *deco)
{
  return load(mem_input_stream(buf, len), *deco);
}


std::unique_ptr<node> load_stream(
    std::istream &is,
    decoder *deco)
{
  return load(cpp_input_stream(is), *deco);
}


}  /* namespace ellis */
