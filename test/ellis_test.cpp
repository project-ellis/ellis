#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <ellis/array_node.hpp>
#include <ellis/binary_node.hpp>
#include <ellis/err.hpp>
#include <ellis/map_node.hpp>
#include <ellis/node.hpp>
#include <ellis/private/using.hpp>
#include <string.h>

void arraytest()
{
  using namespace ellis;
  node en(type::ARRAY);
  auto &a = en.as_array();

  a.append("foo");
  a.append(4);
  a.append(4.4);
  a.append(true);
  a.append(type::NIL);

  assert(a.length() == 5);

  assert(a[0] == "foo");
  assert(a[0] == "foo");
  assert(a[1] == 4);
  assert(a[2] == 4.4);
  assert(a[3] == true);
  assert(a[4] == type::NIL);
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
  // TODO: cas_binary(). assert(b5.as_binary().data() == b1.as_binary().data());
  b5.as_binary().resize(sdlen+1);
  //assert(b5.as_binary().data() != b1.as_binary().data());

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
  assert(en.as_map()["foo"] == 4);
  assert(en.as_map().length() == 1);

  node bar(type::MAP);
  en.as_map().insert("bar", bar);
  assert(en.as_map().has_key("bar"));
  assert(en.as_map()["bar"].get_type() == type::MAP);
  assert(en.as_map()["bar"].is_type(type::MAP));
  assert(en.as_map()["bar"] == bar);

  /* Make sure COW works correctly. */
  bar.as_map().insert("somekey", true);
  assert(bar.as_map()["somekey"] == true);
  assert(! en.as_map()["bar"].as_map().has_key("somekey"));

  vector<string> e_keys;
  e_keys.push_back("foo");
  e_keys.push_back("bar");
  vector<string> a_keys = en.as_map().keys();
  sort(e_keys.begin(), e_keys.end());
  sort(a_keys.begin(), a_keys.end());
  assert(e_keys == a_keys);

  node before = en;
  assert(before == en);

  int foo_count = 0;
  auto count_fn = [&foo_count](const string &k, const node &)
  {
    if (k == "foo") {
      foo_count++;
    }
  };
  en.as_map().foreach(count_fn);
  assert(foo_count == 1);

  auto filter_fn = [](const string &k, const node &v)->bool
  {
    if (k == "foo" && v == 4) {
      return true;
    }
    else {
      return false;
    }
  };
  node filtered = en.as_map().filter(filter_fn);
  assert(filtered.as_map().has_key("foo"));
  assert(filtered.as_map()["foo"] == 4);

  /*
   * Do the destructive tests last to avoid previous tests having to worry about
   * test order.
   */

  node other(type::MAP);
  node child(type::MAP);
  merge_policy policy;
  child.as_map().insert("val", 17);
  other.as_map().insert("foo", "clobbered");
  other.as_map().insert("child", child);

  /* No-op merge. */
  en = before;
  policy.key_exists_copy = false;
  policy.key_missing_copy = false;
  policy.abort_on_not_copy = false;
  en.as_map().merge(other.as_map(), policy);
  assert(before == en);

  /* Exception merge. */
  policy.key_exists_copy = false;
  policy.key_missing_copy = false;
  policy.abort_on_not_copy = true;
  bool raised = false;
  try {
    en.as_map().merge(other.as_map(), policy);
  }
  catch (err e) {
    raised = true;
    assert(e.code() == (int)err_code::NOT_MERGED);
  }
  assert(raised);

  /* Merge only missing keys. */
  policy.key_exists_copy = false;
  policy.key_missing_copy = true;
  policy.abort_on_not_copy = false;
  en.as_map().merge(other.as_map(), policy);
  assert(en.as_map().has_key("child"));
  assert(en.as_map()["child"].as_map().has_key("val"));
  assert(en.as_map()["child"].as_map()["val"] == 17);
  assert(en.as_map()["foo"] == 4);

  /* Merge only existing keys. */
  en = before;
  policy.key_exists_copy = true;
  policy.key_missing_copy = false;
  policy.abort_on_not_copy = false;
  en.as_map().merge(other.as_map(), policy);
  assert(! en.as_map().has_key("child"));
  assert(en.as_map()["foo"] == "clobbered");

  /* Merge all keys. */
  en = before;
  policy.key_exists_copy = true;
  policy.key_missing_copy = true;
  policy.abort_on_not_copy = false;
  en.as_map().merge(other.as_map(), policy);
  assert(en.as_map().has_key("child"));
  assert(en.as_map()["child"].as_map().has_key("val"));
  assert(en.as_map()["child"].as_map()["val"] == 17);
  assert(en.as_map()["foo"] == "clobbered");

  /* Mutable foreach. */
  en = before;
  auto mut_foo_fn = [](const string &k, node &v)
  {
    if (k == "foo") {
      v = "fooval";
    }
  };
  en.as_map().foreach(mut_foo_fn);
  node x = en.as_map()["foo"];
  assert(en.as_map()["foo"] == "fooval");

  assert(en.as_map().has_key("foo"));
  en.as_map().erase("foo");
  assert(! en.as_map().has_key("foo"));

  assert(en.as_map().length() > 0);
  assert(! en.as_map().empty());
  en.as_map().clear();
  assert(en.as_map().length() == 0);
  assert(en.as_map().empty());
}

void pathtest()
{
  using namespace ellis;
  node a(type::ARRAY);
  a.as_array().append(4);
  a.as_array().append("hi");
  node r(type::MAP);
  r.as_map().insert("foo", a);
  assert(r.path("{foo}[0]") == 4);
  assert(r.path("{foo}[1]") == "hi");
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
  /* TODO: generic nodetest */
  arraytest();
  binarytest();
  maptest();
  pathtest();
  printf("all tests completed.\n");
  return 0;
}
