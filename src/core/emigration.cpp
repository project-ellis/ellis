#include <ellis/core/emigration.hpp>

#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>


namespace ellis {


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
      *err_ret = std::move(out->extract_output_error());
      return false;
    }
    /* Have encoder fill the buffer. */
    auto st = enco->fill_buffer(buf, &bytecount);
    /* Emit whatever we were given to emit, regardless of error status. */
    if (! out->emit(bytecount)) {
      *err_ret = std::move(out->extract_output_error());
      return false;
    }
    // TODO: switch to switch
    if (st == encoding_status::END) {
      return true;
    }
    else if (st == encoding_status::ERROR) {
      *err_ret = std::move(enco->extract_error());
    }
    else if (st == encoding_status::CONTINUE) {
      continue;
    }
    else {
      ELLIS_ASSERT_UNREACHABLE();
    }
  }
  ELLIS_ASSERT_UNREACHABLE();
  return nullptr;
}


}  /* namespace ellis */
