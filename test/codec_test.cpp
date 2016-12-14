#undef NDEBUG
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/system.hpp>
#include <ellis/codec/delimited_text.hpp>
#include <ellis/private/using.hpp>
#include <ellis/stream/cpp_input_stream.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <sstream>


int main() {
  using namespace ellis;
  //auto n = load(mem_input_stream("[1,2,3]"), json_decoder());
  //dump(n, file_output_stream("/tmp/lame.json"), json_encoder());
  //file_input_stream is("/tmp/lame.json");
  //json_decoder jd;
  //unique_ptr<node> n1(load(is, jd));
  //unique_ptr<node> n2(load(is, jd));
  std::stringstream ss1;
  ss1 << "one" << std::endl << "two" << std::endl << "three" << std::endl;
  auto s1 = ss1.str();
  auto n = load(cpp_input_stream(ss1), delimited_text_decoder());
  std::stringstream ss2;
  dump(n.get(), cpp_output_stream(ss2), delimited_text_encoder());
  auto s2 = ss2.str();
  ELLIS_ASSERT_EQ(s1, s2);
  return 0;
}
