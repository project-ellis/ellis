#include <ellis/stream/tcp_client_stream.hpp>

#include <ellis/core/defs.hpp>
#include <ellis_private/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


tcp_stream::tcp_stream(UNUSED const char *host, UNUSED const char *port) {
  // int fd = tcpclientopen(host, port);  // if only that easy
  // TODO: throw exception on tcpclientopen
  // m_fdis.reset(new fd_input_stream(fd));
  // m_fdos.reset(new fd_output_stream(fd));
}

tcp_stream::~tcp_stream() {
  // close(m_fd);
}

bool tcp_stream::next_input_buf(byte **buf, size_t *bytecount) {
  return m_fdis->next_input_buf(buf, bytecount);
}

void tcp_stream::put_back(size_t bytecount) {
  return m_fdis->put_back(bytecount);
}

unique_ptr<err> tcp_stream::extract_input_error() {
  return m_fdis->extract_input_error();
}

bool tcp_stream::next_output_buf(byte **buf, size_t *bytecount) {
  return m_fdos->next_output_buf(buf, bytecount);
}

bool tcp_stream::emit(size_t bytecount) {
  return m_fdos->emit(bytecount);
}

unique_ptr<err> tcp_stream::extract_output_error() {
  return m_fdos->extract_output_error();
}


}  /* namespace ellis */
