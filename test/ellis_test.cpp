#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <ellis/ellis_node.hpp>
#include <ellis/ellis_map_node.hpp>

void maptest()
{
  using namespace ellis;
  ellis_node en(ellis_type::MAP);
  en.as_map().insert("foo", (int64_t)4);
  assert(en.as_map()["foo"].as_int64() == 4);
}

int main()
{
  maptest();
  printf("all tests completed.\n");
  return 0;
}
