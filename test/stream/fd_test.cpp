#undef NDEBUG
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>
#include <ellis/stream/fd_input_stream.hpp>
#include <ellis/stream/fd_output_stream.hpp>
#include <sstream>
#include <unistd.h>

void round_trip_test(const char *mem, size_t len)
{
  using namespace ellis;
  int fd[2];
  ELLIS_ASSERT_EQ(pipe(fd), 0);

  /* Write to pipe. */
  fd_output_stream fdos(fd[1]);
  const char *mem_end = mem + len;
  for (const char *p = mem; p < mem_end; ) {
    byte *buf;
    size_t avail;
    ELLIS_ASSERT_TRUE(fdos.next_output_buf(&buf, &avail));
    ELLIS_ASSERT_GT(avail, 0);
    size_t how_much = std::min((size_t)(mem_end - p), avail);
    memcpy(buf, p, how_much);
    fdos.emit(how_much);
    p += how_much;
  }
  close(fd[1]);

  /* Read from pipe. */
  fd_input_stream fdis(fd[0]);
  std::ostringstream got;
  while (1) {
    byte *buf;
    size_t avail;
    if (! fdis.next_input_buf((const byte **)&buf, &avail)) {
      break;
    }
    ELLIS_ASSERT_GT(avail, 0);
    got.write((const char *)buf, avail);
  }
  close(fd[0]);

  /* Compare recreated data. */
  string s = got.str();
  ELLIS_ASSERT_EQ(s.size(), len);
  ELLIS_ASSERT_MEM_EQ((const byte *)(s.data()), (const byte *)mem, len);
}

void round_trip_test(const char *mem)
{
  round_trip_test(mem, strlen(mem));
}

int main() {
  using namespace ellis;
  round_trip_test("");
  round_trip_test("'");
  round_trip_test("0");
  round_trip_test("something\nhere\n");
  char buf[20000];
  for (size_t i = 0; i < sizeof(buf); ++i) {
    buf[i] = 'A' + (i % 13);
  }
  round_trip_test(buf, 1000);
  round_trip_test(buf, 4095);
  round_trip_test(buf, 4096);
  round_trip_test(buf, 4097);
  round_trip_test(buf, 20000);
  return 0;
}
