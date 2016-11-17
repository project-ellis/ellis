#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <ellis/ellis_array_node.hpp>
#include <ellis/ellis_map_node.hpp>
#include <ellis/ellis_node.hpp>

void arraytest()
{
  using namespace ellis;
  node en(type::ARRAY);

  node foo(std::string("foo"));
  en.as_array().append(foo);

  node four((int64_t)4);
  en.as_array().append(four);

  assert(en.as_array()[0].as_u8str() == foo.as_u8str());
  assert(en.as_array()[1].as_int64() == four.as_int64());
}

void maptest()
{
  using namespace ellis;
  node en(type::MAP);
  node foo(std::string("foo"));
  node four((int64_t)4);
  en.as_map().insert(foo.as_u8str(), four);
  assert(en.as_map()[foo.as_u8str()].as_int64() == four.as_int64());
}

int main()
{
  arraytest();
  maptest();
  printf("all tests completed.\n");
  return 0;
}
