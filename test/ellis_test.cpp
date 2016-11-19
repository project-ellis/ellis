#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <ellis/array_node.hpp>
#include <ellis/binary_node.hpp>
#include <ellis/err.hpp>
#include <ellis/map_node.hpp>
#include <ellis/node.hpp>
#include <string.h>

void arraytest()
{
  using namespace ellis;
  node en(type::ARRAY);
  auto &a = en.as_array();

  a.append(node("foo"));
  a.append(node(4));
  a.append(node(4.4));
  a.append(node(true));

  assert(a.length() == 4);

  assert(a[0].as_u8str() == "foo");
  assert(a[0] == "foo");
  assert(a[1].as_int64() == 4);
  assert(a[2].as_double() == 4.4);
  assert(a[3].as_bool() == true);
}

void binarytest()
{
  using namespace ellis;
  const uint8_t somedata[] = {
    0x00, 0x81, 0x23, 0xE8,
    0xD8, 0xAF, 0xF7, 0x00
  };
  size_t sdlen = sizeof(somedata);
  /* Node b1 is constructed directly from the binary data. */
  node b1(somedata, sdlen);
  /* Node b2 is copy constructed. */
  node b2(b1);
  /* Node b3 is built up by appending. */
  node b3(type::BINARY);
  b3.as_binary().append(somedata, sdlen);
  /* Node b4 has same length but is not equal. */
  node b4(type::BINARY);
  b4.as_binary().resize(sdlen);
  /* Node b5 starts out equal to b1 but is extended with a zero. */
  node b5(b1);
  b5.as_binary().resize(sdlen+1);

  auto fn = [&b1, &b4, &b5, &somedata, &sdlen](const node &n)
  {
    const auto &b = n.as_binary();
    assert(! b.empty());
    assert(b.length() == sdlen);
    assert(b.data() != nullptr);
    assert(b.data() != somedata);
    assert(memcmp(b.data(), somedata, sdlen) == 0);
    assert(n == b1);
    assert(n != b4);
    assert(n != b5);
    assert(b[0] == b[7]);
    assert(b[1] == 0x81);
  };

  fn(b1);
  fn(b2);
  fn(b3);

  size_t big = 1024;
  b4.as_binary().resize(big);

  auto zchk = [](const node &n, size_t len)
  {
    const auto &b = n.as_binary();
    assert(b.length() == len);
    const uint8_t *p = b.data();
    for (size_t i = 0; i < len; i++) {
      assert(p[i] == 0);
      assert(b[i] == 0);
    }
  };

  zchk(b4, big);
  b4.as_binary().resize(big - 1);
  zchk(b4, big - 1);
}

void maptest()
{
  using namespace ellis;
  node en(type::MAP);
  en.as_map().insert("foo", 4);
  assert(en.as_map()["foo"].as_int64() == 4);
}

void pathtest()
{
  using namespace ellis;
  node a(type::ARRAY);
  a.as_array().append(4);
  a.as_array().append("hi");
  node r(type::MAP);
  r.as_map().insert("foo", a);
  assert(r.path("{foo}[0]").as_int64() == 4);
  assert(r.path("{foo}[1]").as_u8str() == "hi");
  auto chk_fail = [&r](const char *path)
  {
    bool threw = false;
    try {
      r.path(path);
    } catch(err e) {
      threw = true;
    }
    assert(threw);
  };
  chk_fail("?");
  chk_fail("{foo");
  chk_fail("{bar}");
  chk_fail("[x]");
  chk_fail("[0]");
  chk_fail("{foo}{bar}");
  chk_fail("{foo}[2]");
  chk_fail("{foo}[0][1]");
  chk_fail("{foo}[0]{1}");
}

int main()
{
  arraytest();
  binarytest();
  maptest();
  pathtest();
  printf("all tests completed.\n");
  return 0;
}
