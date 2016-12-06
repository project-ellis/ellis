#undef NDEBUG
#include <assert.h>
#include <cfloat>
#include <ellis/array_node.hpp>
#include <ellis/binary_node.hpp>
#include <ellis/err.hpp>
#include <ellis/map_node.hpp>
#include <ellis/node.hpp>
#include <ellis/private/using.hpp>
#include <stdio.h>
#include <string.h>

static const uint8_t k_somedata[] = {
  0x00, 0x81, 0x23, 0xE8,
  0xD8, 0xAF, 0xF7, 0x00
};

void primitivetest()
{
  using namespace ellis;
  node v1(2.1);
  assert(v1.as_double() == 2.1);
  double d1 = (double)v1;
  assert(d1 == 2.1);

  /* Comparison. */
  v1 = 2;
  assert(v1 < 3);
  assert(v1 <= 2);
  assert(v1 <= 3);
  assert(v1 > 1);
  assert(v1 >= 2);
  assert(v1 >= 1);

  v1 = 2.0;
  assert(v1 < 3.0);
  assert(v1 <= 2.0);
  assert(v1 <= 3.0);
  assert(v1 > 1.0);
  assert(v1 >= 2.0);
  assert(v1 >= 1.0);

  auto type_fail = [](std::function<void()> fn)
  {
    bool threw = false;
    try {
      fn();
    } catch(err e) {
      if (e.code() == err_code::WRONG_TYPE) {
        threw = true;
      }
    }
    assert(threw);
  };

  v1 = 1;
  type_fail([&v1]()
    {
      /* Use printf to make sure the line does not get optimized out. */
      printf("%d", v1 < 2.0);
    });
  v1 = 1.0;
  type_fail([&v1]()
    {
      printf("%d", v1 < 2);
    });

  /* Arithmetic. */
  v1 = 2;
  assert(v1 == 2);
  assert(v1 + 1 == 3);
  assert(1 + v1 == 3);
  assert(v1 - 1 == 1);
  assert(1 - v1 == -1);
  assert(v1 * 2 == 4);
  assert(2 * v1 == 4);
  assert(v1 / 2 == 1);
  assert(2 / v1 == 1);
  v1 += 1;
  assert(v1 == 3);
  v1 -= 1;
  assert(v1 == 2);
  v1 *= 2;
  assert(v1 == 4);
  v1 /= 2;
  assert(v1 == 2);

  auto dbl_equal = [](double a, double b)
  {
    return abs(a - b) <= DBL_EPSILON;
  };

  v1 = 2.0;
  assert(v1 == 2.0);
  assert(dbl_equal(v1 + 1.0, 3.0));
  assert(dbl_equal(1.0 + v1, 3.0));
  assert(dbl_equal(v1 - 1.0, 1.0));
  assert(dbl_equal(1.0 - v1, -1.0));
  assert(dbl_equal(v1 * 2.0, 4.0));
  assert(dbl_equal(2.0 * v1, 4.0));
  assert(dbl_equal(v1 / 2.0, 1.0));
  assert(dbl_equal(2.0 / v1, 1.0));
  v1 += 1.0;
  assert(dbl_equal(v1, 3.0));
  v1 -= 1.0;
  assert(dbl_equal(v1, 2.0));
  v1 *= 2.0;
  assert(dbl_equal(v1, 4.0));
  v1 /= 2.0;
  assert(dbl_equal(v1, 2.0));

  v1 = 1;
  type_fail([&v1]()
    {
      node v2 = v1 + 1.0;
    });
  type_fail([&v1]()
    {
      v1 += 1.0;
    });
  v1 = 1.0;
  type_fail([&v1]()
    {
      node v2 = v1 + 1;
    });
  type_fail([&v1]()
    {
      v1 += 1;
    });
}

void strtest()
{
  using namespace ellis;
  node n1("hello");
  node n2(n1);
  assert(n1 == "hello");
  string s1 = (string)n1;
  assert(s1 == "hello");
  const char *cp1 = (const char *)n1;
  const char *cp2 = (const char *)n1;
  assert(cp1 == cp2);
  assert(strcmp(cp1, "hello") == 0);
  n1.as_mutable_u8str().append(" world");
  assert(n1 == "hello world");
  assert(n2 != "hello world");
}

void arraytest()
{
  using namespace ellis;
  node en(type::ARRAY);
  auto &am = en.as_mutable_array();
  const auto &a = en.as_array();

  /* Behavior checks for an empty array. */
  auto empty_chk = [] (const node &n) {
    assert(n.is_type(type::ARRAY));
    const auto &ac = n.as_array();
    assert(ac.length() == 0);
    assert(ac.is_empty());
    bool hit = false;
    ac.foreach([&hit](const node &){hit = true;});
    assert(hit == false);
    auto f = ac.filter([](const node &){return true;});
    assert(f.as_array().is_empty());
    try {
      ac[0];
    } catch (std::out_of_range) {
      hit = true;
    }
    assert(hit == true);
    node n2(type::ARRAY);
    assert(n == n2);
  };

  empty_chk(en);

  am.reserve(7);
  am.append("foo");
  am.append("bar");
  am.erase(1);
  am.insert(0, "world");
  am.erase(0);
  am.insert(1, 4.4);
  am.append(true);
  am.append(type::NIL);
  am.append(type::ARRAY);
  am.append(type::MAP);
  am.append(node(k_somedata, sizeof(k_somedata)));
  am.insert(1, 4);

  /* Behavior checks for the custom array we just built. */
  auto contents_chk = [] (const node &n) {
    assert(n.is_type(type::ARRAY));
    const auto &ac = n.as_array();
    assert(ac.length() == 8);
    assert(!ac.is_empty());
    assert(ac[0] == "foo");
    assert(ac[1] == 4);
    assert(ac[2] == 4.4);
    assert(ac[3] == true);
    assert(ac[4] == type::NIL);
    assert(ac[5].get_type() == type::ARRAY);
    assert(ac[6].get_type() == type::MAP);
    assert(ac[7].get_type() == type::BINARY);
    assert(ac[7].as_binary().length() == sizeof(k_somedata));
    std::map<type, int> tc;
    ac.foreach([&tc](const node &x) {
        tc[x.get_type()]++;
      });
    assert(tc.size() == 8);
    assert(ac.filter([](const node &){return true;}) .as_array().length() == 8);
    assert(ac.filter([](const node &){return false;}).as_array().length() == 0);
    for (auto t : { type::NIL , type::BOOL , type::INT64 , type::DOUBLE,
                    type::U8STR , type::ARRAY , type::BINARY , type::MAP }) {
      assert(ac.filter(
            [t](const node &x)
            {
              return x.get_type() == t;
            }).as_array().length() == 1);
    }
  };

  /* Copy the newly filled array. */
  node en2(en);
  assert(en.as_array()[0].as_u8str().c_str()
      == en2.as_array()[0].as_u8str().c_str());
  contents_chk(en);
  contents_chk(en2);
  /* Clear the original, verify the copy looks right and the original empty. */
  en.as_mutable_array().clear();
  empty_chk(en);
  contents_chk(en2);
  /* Swap, and verify the other way around. */
  en.swap(en2);
  empty_chk(en2);
  contents_chk(en);
  /* Reset the copy to original contents, and try deleting from left. */
  en2 = en;
  for (int i = 7; i >= 0; i--) {
    en2.as_mutable_array().erase(0);
    assert(en2.as_array().length() == (size_t)i);
  }
  empty_chk(en2);
  /* Reset the copy to original contents, and try deleting from right. */
  en2 = en;
  for (int i = 7; i >= 0; i--) {
    en2.as_mutable_array().erase(i);
    assert(en2.as_array().length() == (size_t)i);
  }
  empty_chk(en2);
  /* Make sure the original is still intact. */
  contents_chk(en);
}

void binarytest()
{
  using namespace ellis;
  size_t sdlen = sizeof(k_somedata);
  /* Node b1 is constructed directly from the binary data. */
  node b1(k_somedata, sdlen);
  /* Node b2 is copy constructed. */
  node b2(b1);
  /* Node b3 is built up by appending. */
  node b3(type::BINARY);
  b3.as_mutable_binary().append(k_somedata, sdlen);
  /* Node b4 has same length but is not equal. */
  node b4(type::BINARY);
  b4.as_mutable_binary().resize(sdlen);
  /* Node b5 starts out equal to b1 but is extended with a zero. */
  node b5(b1);
  assert(b5.as_binary().data() == b1.as_binary().data());
  b5.as_mutable_binary().resize(sdlen+1);
  assert(b5.as_binary().data() != b1.as_binary().data());

  auto fn = [&b1, &b4, &b5, &sdlen](const node &n)
  {
    const auto &b = n.as_binary();
    assert(! b.is_empty());
    assert(b.length() == sdlen);
    assert(b.data() != nullptr);
    assert(b.data() != k_somedata);
    assert(memcmp(b.data(), k_somedata, sdlen) == 0);
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
  b4.as_mutable_binary().resize(big);

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
  b4.as_mutable_binary().resize(big - 1);
  zchk(b4, big - 1);
}

void maptest()
{
  using namespace ellis;
  node en(type::MAP);
  en.as_mutable_map().replace("foo", 5);  // should fail, nothing to replace
  assert(! en.as_map().has_key("foo"));   // no such key
  en.as_mutable_map().insert("foo", 4);
  assert(en.as_map().has_key("foo"));     // now the key is there
  en.as_mutable_map().insert("foo", 72);  // no effect here
  assert(en.as_map()["foo"] == 4);
  assert(en.as_map().length() == 1);

  node bar(type::MAP);
  en.as_mutable_map().set("bar", bar);
  assert(en.as_map().has_key("bar"));
  assert(en.as_map()["bar"].get_type() == type::MAP);
  assert(en.as_map()["bar"].is_type(type::MAP));
  assert(en.as_map()["bar"] == bar);

  /* Make sure COW works correctly. */
  bar.as_mutable_map().insert("somekey", true);
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
  child.as_mutable_map().insert("val", 17);
  other.as_mutable_map().insert("foo", "clobbered");
  other.as_mutable_map().insert("child", child);

  /* Merge fail function. */
  en = before;
  bool did_fail = false;
  std::function<void(const std::string &, const node &)> failfn =
    [&did_fail] (const std::string &, const node &) {
      did_fail = true;
    };
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_ONLY, &failfn);
  assert(did_fail);
  did_fail = false;
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_OR_REPLACE,
      &failfn);
  assert(!did_fail);


  /* Merge only missing keys. */
  en = before;
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_ONLY, nullptr);
  assert(en.as_map().has_key("child"));
  assert(en.as_map()["child"].as_map().has_key("val"));
  assert(en.as_map()["child"].as_map()["val"] == 17);
  assert(en.as_map()["foo"] == 4);

  /* Merge only existing keys. */
  en = before;
  en.as_mutable_map().merge(other.as_map(), add_policy::REPLACE_ONLY, nullptr);
  assert(! en.as_map().has_key("child"));
  assert(en.as_map()["foo"] == "clobbered");

  /* Merge all keys. */
  en = before;
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_OR_REPLACE,
      nullptr);
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
  en.as_mutable_map().foreach_mutable(mut_foo_fn);
  node x = en.as_map()["foo"];
  assert(en.as_map()["foo"] == "fooval");

  assert(en.as_map().has_key("foo"));
  en.as_mutable_map().erase("foo");
  assert(! en.as_map().has_key("foo"));

  assert(en.as_map().length() > 0);
  assert(! en.as_map().is_empty());
  en.as_mutable_map().clear();
  assert(en.as_map().length() == 0);
  assert(en.as_map().is_empty());
}

void pathtest()
{
  using namespace ellis;
  node a(type::ARRAY);
  a.as_mutable_array().append(4);
  a.as_mutable_array().append("hi");
  node r(type::MAP);
  r.as_mutable_map().insert("foo", a);
  assert(r.at_path("{foo}[0]") == 4);
  assert(r.at_path("{foo}[1]") == "hi");
  auto chk_fail = [&r](const char *path)
  {
    bool threw = false;
    try {
      r.at_path(path);
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
  r.at_path_mutable("{foo}[0]") = 5;
  assert(r.at_path("{foo}[0]") == 5);
}

int main()
{
  /* TODO: generic nodetest */
  primitivetest();
  strtest();
  arraytest();
  binarytest();
  maptest();
  pathtest();
  printf("all tests completed.\n");
  return 0;
}
