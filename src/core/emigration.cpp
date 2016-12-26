#include <ellis/core/emigration.hpp>

#include <ellis/core/system.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <ellis/stream/fd_output_stream.hpp>
#include <ellis/stream/file_output_stream.hpp>
#include <ellis/stream/mem_output_stream.hpp>
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



}  /* namespace ellis */
