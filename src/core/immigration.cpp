#include <ellis/core/immigration.hpp>

#include <ellis/core/system.hpp>
#include <ellis/stream/cpp_input_stream.hpp>
#include <ellis/stream/fd_input_stream.hpp>
#include <ellis/stream/file_input_stream.hpp>
#include <ellis/stream/mem_input_stream.hpp>
#include <ellis_private/convenience/file.hpp>
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
      st = deco->chop();
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


std::unique_ptr<node> load_file_autodecode(
    const char *filename)
{
  string exten = get_extension(filename);
  string failmsg = "no decoders found";

  auto fmts = system_lookup_data_formats_by_extension(exten.c_str());
  if (fmts.empty()) {
    THROW_ELLIS_ERR(NO_SUCH,
        "No valid format found for input extension (" << exten << ")");
  }

  unique_ptr<node> nod;
  for (auto fmt: fmts) {
    auto dec = (fmt->m_make_decoder)();
    if (!dec) {
      /* No decoder function; move on to the next format. */
      continue;
    }
    try {
      nod = load_file(filename, dec.get());
    }
    catch (const err &e) {
      /* Failed decode; remember error, but move on to the next format. */
      failmsg = e.msg();
    }
  }
  if (! nod) {
    THROW_ELLIS_ERR(PARSE_FAIL,
        "Unable to decode file (" << filename << "): " << failmsg);
  }
  return nod;
}


}  /* namespace ellis */
