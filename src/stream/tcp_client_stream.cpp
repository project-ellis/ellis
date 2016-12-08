#include <ellis/stream/tcp_client_stream.hpp>

#include <ellis/private/err.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


tcp_stream::tcp_stream(const char *, const char *) {
  // int fd = tcpclientopen(host, port);  // if only that easy
  // TODO: throw exception on tcpclientopen
  // m_fdis.reset(new fd_input_stream(fd));
  // m_fdos.reset(new fd_output_stream(fd));
}

tcp_stream::~tcp_stream() {
  // close(m_fd);
}

bool tcp_stream::next_input_buf(byte **buf, int *bytecount) {
  return m_fdis->next_input_buf(buf, bytecount);
}

void tcp_stream::put_back(int bytecount) {
  return m_fdis->put_back(bytecount);
}

unique_ptr<err> tcp_stream::get_input_error() {
  return m_fdis->get_input_error();
}

bool tcp_stream::next_output_buf(byte **buf, int *bytecount) {
  return m_fdos->next_output_buf(buf, bytecount);
}

bool tcp_stream::emit(int bytecount) {
  return m_fdos->emit(bytecount);
}

unique_ptr<err> tcp_stream::get_output_error() {
  return m_fdos->get_output_error();
}


}  /* namespace ellis */
