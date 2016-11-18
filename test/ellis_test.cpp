#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <ellis/array_node.hpp>
#include <ellis/map_node.hpp>
#include <ellis/node.hpp>

void arraytest()
{
  using namespace ellis;
  node en(type::ARRAY);
  auto &a = en.as_array();

  a.append(node(std::string("foo")));
  a.append(node((int64_t)4));
  a.append(node((double)4.4));

  assert(a.length() == 3);

  assert(a[0].as_u8str() == "foo");
  assert(a[1].as_int64() == 4);
  assert(a[2].as_double() == 4.4);
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
