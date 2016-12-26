#include <ellis/core/emigration.hpp>

#include <ellis/core/system.hpp>
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


}  /* namespace ellis */
