#undef NDEBUG
#define ELLIS_DISABLE_UNREACHABLE_HINT
#include <assert.h>
#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>
#include <stdio.h>
#include <string.h>

static const ellis::byte k_somedata[] = {
  0x00, 0x81, 0x23, 0xE8,
  0xD8, 0xAF, 0xF7, 0x00
};


/* Used by logtest. */
static int g_log_called = 0;

/* Used by logtest. */
static void test_log_function(
    UNUSED ellis::log_severity sev,
    UNUSED const char *file,
    UNUSED int line,
    UNUSED const char *func,
    UNUSED const char *fmt, ...)
{
  g_log_called++;
}


static void logtest()
{
  /* By the way, we use assert instead of ELLIS_ASSERT in this function, since
   * the latter may itself cause logging, and thus add to confusion of the
   * logging test. */

  auto cause_logs = []() {
    ELLIS_LOG(INFO, "hello no args");
    ELLIS_LOG(DBUG, "errno was %d", errno);
    ELLIS_LOGSTREAM(DBUG, "errno was " << errno);
  };

  /* Nothing bad happening by default with logging. */
  cause_logs();

  /* Replace log function, observe behavior. */
  ellis::set_system_log_function(test_log_function);
  cause_logs();
  assert(g_log_called == 0);

  /* Adjust prefilter to INFO, try again. */
  ellis::set_system_log_prefilter(ellis::log_severity::INFO);
  cause_logs();
  assert(g_log_called == 1);
  g_log_called = 0;

  /* Adjust prefilter to DBUG, should see all. */
  ellis::set_system_log_prefilter(ellis::log_severity::DBUG);
  cause_logs();
  assert(g_log_called == 3);
}


/* Used by asserttest. */
static int g_crash_called = 0;

/* Used by asserttest. */
static void test_crash_function(
    UNUSED const char *file,
    UNUSED int line,
    UNUSED const char *func,
    UNUSED const char *fmt, ...)
{
  g_crash_called++;
}


static void asserttest()
{
  auto safe_asserts = []() {
    int x = 1;
    ELLIS_ASSERT_TRUE(true);
    ELLIS_ASSERT_GT(x, 0);
    if (x == 1) {
    } else {
      ELLIS_ASSERT_UNREACHABLE();
    }
  };
  int x = 0;
  auto unsafe_asserts = [&x]() {
    ELLIS_ASSERT_TRUE(false);
    x++;
    ELLIS_ASSERT_EQ(x, 0);
    x++;
    if (x != 0) {
      ELLIS_ASSERT_UNREACHABLE();
    }
    x++;
    ELLIS_CRASH("the murderer was %s", "aaaaaaaa");
    x++;
  };

  /* Try some safe asserts using default crash handler. */
  safe_asserts();
  assert(g_crash_called == 0);

  /* Now replace crash function and try again with safe asserts. */
  ellis::set_system_crash_function(&test_crash_function);
  safe_asserts();
  assert(g_crash_called == 0);

  /* Now for the unsafe asserts. */
  unsafe_asserts();
  assert(g_crash_called == x);
}

static void primitivetest()
{
  using namespace ellis;
  node v1(2.1);
  ELLIS_ASSERT_EQ(v1.as_double(), 2.1);
  double d1 = (double)v1;
  ELLIS_ASSERT_EQ(d1, 2.1);

  /* Comparison. */
  v1 = 2;
  ELLIS_ASSERT_LT(v1, 3);
  ELLIS_ASSERT_LTE(v1, 2);
  ELLIS_ASSERT_LTE(v1, 3);
  ELLIS_ASSERT_GT(v1, 1);
  ELLIS_ASSERT_GTE(v1, 2);
  ELLIS_ASSERT_GTE(v1, 1);

  v1 = 2.0;
  ELLIS_ASSERT_LT(v1, 3.0);
  ELLIS_ASSERT_LTE(v1, 2.0);
  ELLIS_ASSERT_LTE(v1, 3.0);
  ELLIS_ASSERT_GT(v1, 1.0);
  ELLIS_ASSERT_GTE(v1, 2.0);
  ELLIS_ASSERT_GTE(v1, 1.0);

  auto type_fail = [](std::function<void()> fn)
  {
    bool threw = false;
    try {
      fn();
    } catch(const err &e) {
      if (e.code() == err_code::WRONG_TYPE) {
        threw = true;
      }
    }
    ELLIS_ASSERT_TRUE(threw);
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
  ELLIS_ASSERT_EQ(v1, 2);
  ELLIS_ASSERT_EQ(v1 + 1, 3);
  ELLIS_ASSERT_EQ(1 + v1, 3);
  ELLIS_ASSERT_EQ(v1 - 1, 1);
  ELLIS_ASSERT_EQ(1 - v1, -1);
  ELLIS_ASSERT_EQ(v1 * 2, 4);
  ELLIS_ASSERT_EQ(2 * v1, 4);
  ELLIS_ASSERT_EQ(v1 / 2, 1);
  ELLIS_ASSERT_EQ(2 / v1, 1);
  v1 += 1;
  ELLIS_ASSERT_EQ(v1, 3);
  v1 -= 1;
  ELLIS_ASSERT_EQ(v1, 2);
  v1 *= 2;
  ELLIS_ASSERT_EQ(v1, 4);
  v1 /= 2;
  ELLIS_ASSERT_EQ(v1, 2);

  v1 = 2.0;
  ELLIS_ASSERT_EQ(v1, 2.0);
  ELLIS_ASSERT_TRUE(dbl_equal(v1 + 1.0, 3.0));
  ELLIS_ASSERT_TRUE(dbl_equal(1.0 + v1, 3.0));
  ELLIS_ASSERT_TRUE(dbl_equal(v1 - 1.0, 1.0));
  ELLIS_ASSERT_TRUE(dbl_equal(1.0 - v1, -1.0));
  ELLIS_ASSERT_TRUE(dbl_equal(v1 * 2.0, 4.0));
  ELLIS_ASSERT_TRUE(dbl_equal(2.0 * v1, 4.0));
  ELLIS_ASSERT_TRUE(dbl_equal(v1 / 2.0, 1.0));
  ELLIS_ASSERT_TRUE(dbl_equal(2.0 / v1, 1.0));
  v1 += 1.0;
  ELLIS_ASSERT_TRUE(dbl_equal(v1, 3.0));
  v1 -= 1.0;
  ELLIS_ASSERT_TRUE(dbl_equal(v1, 2.0));
  v1 *= 2.0;
  ELLIS_ASSERT_TRUE(dbl_equal(v1, 4.0));
  v1 /= 2.0;
  ELLIS_ASSERT_TRUE(dbl_equal(v1, 2.0));

  v1 = 1;
  node v2(2);
  v1 += v2;
  ELLIS_ASSERT_EQ(v1, 3);

  v1 = 1.0;
  v2 = 2.0;
  v1 += v2;
  ELLIS_ASSERT_TRUE(dbl_equal(v1, 3.0));

  v1 = 1;
  type_fail([&v1]()
    {
      node v3 = v1 + 1.0;
    });
  type_fail([&v1]()
    {
      v1 += 1.0;
    });
  v1 = 1.0;
  type_fail([&v1]()
    {
      node v3 = v1 + 1;
    });
  type_fail([&v1]()
    {
      v1 += 1;
    });
}

static void strtest()
{
  using namespace ellis;
  node n1("hello");
  node n2(n1);
  ELLIS_ASSERT_EQ(n1, "hello");
  string s1 = (string)n1;
  ELLIS_ASSERT_EQ(s1, "hello");
  const char *cp1 = (const char *)n1;
  const char *cp2 = (const char *)n1;
  ELLIS_ASSERT_EQ(cp1, cp2);
  ELLIS_ASSERT_EQ(strcmp(cp1, "hello"), 0);
  n1.as_mutable_u8str().append(" world");
  ELLIS_ASSERT_EQ(n1, "hello world");
  ELLIS_ASSERT_NEQ(n2, "hello world");
}

static void arraytest()
{
  using namespace ellis;
  node en(type::ARRAY);
  auto &am = en.as_mutable_array();

  /* Behavior checks for an empty array. */
  auto empty_chk = [] (const node &n) {
    ELLIS_ASSERT_TRUE(n.is_type(type::ARRAY));
    const auto &ac = n.as_array();
    ELLIS_ASSERT_EQ(ac.length(), 0);
    ELLIS_ASSERT_TRUE(ac.is_empty());
    bool hit = false;
    ac.foreach([&hit](const node &){hit = true;});
    ELLIS_ASSERT_FALSE(hit);
    auto f = ac.filter([](const node &){return true;});
    ELLIS_ASSERT_TRUE(f.as_array().is_empty());
    try {
      ac[0];
    } catch (const std::out_of_range &) {
      hit = true;
    }
    ELLIS_ASSERT_EQ(hit, true);
    node n2(type::ARRAY);
    ELLIS_ASSERT_EQ(n, n2);
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
    ELLIS_ASSERT_TRUE(n.is_type(type::ARRAY));
    const auto &ac = n.as_array();
    ELLIS_ASSERT_EQ(ac.length(), 8);
    ELLIS_ASSERT_TRUE(!ac.is_empty());
    ELLIS_ASSERT_EQ(ac[0], "foo");
    ELLIS_ASSERT_EQ(ac[1], 4);
    ELLIS_ASSERT_EQ(ac[2], 4.4);
    ELLIS_ASSERT_EQ(ac[3], true);
    ELLIS_ASSERT_EQ(ac[4], type::NIL);
    ELLIS_ASSERT_EQ(ac[5].get_type(), type::ARRAY);
    ELLIS_ASSERT_EQ(ac[6].get_type(), type::MAP);
    ELLIS_ASSERT_EQ(ac[7].get_type(), type::BINARY);
    ELLIS_ASSERT_EQ(ac[7].as_binary().length(), sizeof(k_somedata));
    std::map<type, int> tc;
    ac.foreach([&tc](const node &x) {
        tc[x.get_type()]++;
      });
    ELLIS_ASSERT_EQ(tc.size(), 8);
    ELLIS_ASSERT_EQ(ac.filter([](const node &){return true;}) .as_array().length(), 8);
    ELLIS_ASSERT_EQ(ac.filter([](const node &){return false;}).as_array().length(), 0);
    for (auto t : { type::NIL , type::BOOL , type::INT64 , type::DOUBLE,
                    type::U8STR , type::ARRAY , type::BINARY , type::MAP }) {
      ELLIS_ASSERT_EQ(ac.filter(
            [t](const node &x)
            {
              return x.get_type() == t;
            }).as_array().length(), 1);
    }
  };

  /* Copy the newly filled array. */
  node en2(en);
  ELLIS_ASSERT_EQ(en.as_array()[0].as_u8str().c_str(),
      en2.as_array()[0].as_u8str().c_str());
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
    ELLIS_ASSERT_EQ(en2.as_array().length(), (size_t)i);
  }
  empty_chk(en2);
  /* Reset the copy to original contents, and try deleting from right. */
  en2 = en;
  for (int i = 7; i >= 0; i--) {
    en2.as_mutable_array().erase(i);
    ELLIS_ASSERT_EQ(en2.as_array().length(), (size_t)i);
  }
  empty_chk(en2);
  /* Make sure the original is still intact. */
  contents_chk(en);
}

static void binarytest()
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
  ELLIS_ASSERT_EQ(b5.as_binary().data(), b1.as_binary().data());
  b5.as_mutable_binary().resize(sdlen+1);
  ELLIS_ASSERT_NEQ(b5.as_binary().data(), b1.as_binary().data());

  auto fn = [&b1, &b4, &b5, &sdlen](const node &n)
  {
    const auto &b = n.as_binary();
    ELLIS_ASSERT_FALSE(b.is_empty());
    ELLIS_ASSERT_EQ(b.length(), sdlen);
    ELLIS_ASSERT_NULL(b.data());
    ELLIS_ASSERT_NEQ(b.data(), k_somedata);
    ELLIS_ASSERT_EQ(memcmp(b.data(), k_somedata, sdlen), 0);
    ELLIS_ASSERT_EQ(n, b1);
    ELLIS_ASSERT_NEQ(n, b4);
    ELLIS_ASSERT_NEQ(n, b5);
    ELLIS_ASSERT_EQ(b[0], b[7]);
    ELLIS_ASSERT_EQ(b[1],  0x81);
  };

  fn(b1);
  fn(b2);
  fn(b3);

  size_t big = 1024;
  b4.as_mutable_binary().resize(big);

  auto zchk = [](const node &n, size_t len)
  {
    const auto &b = n.as_binary();
    ELLIS_ASSERT_EQ(b.length(), len);
    const byte *p = b.data();
    for (size_t i = 0; i < len; i++) {
      ELLIS_ASSERT_EQ(p[i], 0);
      ELLIS_ASSERT_EQ(b[i], 0);
    }
  };

  zchk(b4, big);
  b4.as_mutable_binary().resize(big - 1);
  zchk(b4, big - 1);
}

static void maptest()
{
  using namespace ellis;
  node en(type::MAP);
  en.as_mutable_map().replace("foo", 5);  // should fail, nothing to replace
  ELLIS_ASSERT_FALSE(en.as_map().has_key("foo"));   // no such key
  en.as_mutable_map().insert("foo", 4);
  ELLIS_ASSERT_TRUE(en.as_map().has_key("foo"));     // now the key is there
  en.as_mutable_map().insert("foo", 72);  // no effect here
  ELLIS_ASSERT_EQ(en.as_map()["foo"], 4);
  ELLIS_ASSERT_EQ(en.as_map().length(), 1);

  node bar(type::MAP);
  en.as_mutable_map().set("bar", bar);
  ELLIS_ASSERT_TRUE(en.as_map().has_key("bar"));
  ELLIS_ASSERT_TRUE(en.as_map()["bar"].get_type() == type::MAP);
  ELLIS_ASSERT_TRUE(en.as_map()["bar"].is_type(type::MAP));
  ELLIS_ASSERT_EQ(en.as_map()["bar"], bar);

  /* Make sure COW works correctly. */
  bar.as_mutable_map().insert("somekey", true);
  ELLIS_ASSERT_TRUE(bar.as_map()["somekey"]);
  ELLIS_ASSERT_FALSE(en.as_map()["bar"].as_map().has_key("somekey"));

  vector<string> e_keys;
  e_keys.push_back("foo");
  e_keys.push_back("bar");
  vector<string> a_keys = en.as_map().keys();
  sort(e_keys.begin(), e_keys.end());
  sort(a_keys.begin(), a_keys.end());
  ELLIS_ASSERT_EQ(e_keys.data(), a_keys.data());

  node before = en;
  ELLIS_ASSERT_EQ(before, en);

  int foo_count = 0;
  auto count_fn = [&foo_count](const string &k, const node &)
  {
    if (k == "foo") {
      foo_count++;
    }
  };
  en.as_map().foreach(count_fn);
  ELLIS_ASSERT_EQ(foo_count, 1);

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
  ELLIS_ASSERT_TRUE(filtered.as_map().has_key("foo"));
  ELLIS_ASSERT_EQ(filtered.as_map()["foo"], 4);

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
  ELLIS_ASSERT_TRUE(did_fail);
  did_fail = false;
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_OR_REPLACE,
      &failfn);
  ELLIS_ASSERT_FALSE(did_fail);


  /* Merge only missing keys. */
  en = before;
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_ONLY, nullptr);
  ELLIS_ASSERT_TRUE(en.as_map().has_key("child"));
  ELLIS_ASSERT_TRUE(en.as_map()["child"].as_map().has_key("val"));
  ELLIS_ASSERT_EQ(en.as_map()["child"].as_map()["val"], 17);
  ELLIS_ASSERT_EQ(en.as_map()["foo"], 4);

  /* Merge only existing keys. */
  en = before;
  en.as_mutable_map().merge(other.as_map(), add_policy::REPLACE_ONLY, nullptr);
  ELLIS_ASSERT_FALSE(en.as_map().has_key("child"));
  ELLIS_ASSERT_EQ(en.as_map()["foo"], "clobbered");

  /* Merge all keys. */
  en = before;
  en.as_mutable_map().merge(other.as_map(), add_policy::INSERT_OR_REPLACE,
      nullptr);
  ELLIS_ASSERT_TRUE(en.as_map().has_key("child"));
  ELLIS_ASSERT_TRUE(en.as_map()["child"].as_map().has_key("val"));
  ELLIS_ASSERT_EQ(en.as_map()["child"].as_map()["val"], 17);
  ELLIS_ASSERT_EQ(en.as_map()["foo"], "clobbered");

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
  ELLIS_ASSERT_EQ(en.as_map()["foo"], "fooval");

  ELLIS_ASSERT_TRUE(en.as_map().has_key("foo"));
  en.as_mutable_map().erase("foo");
  ELLIS_ASSERT_FALSE(en.as_map().has_key("foo"));

  ELLIS_ASSERT_GT(en.as_map().length(), 0);
  ELLIS_ASSERT_FALSE(en.as_map().is_empty());
  en.as_mutable_map().clear();
  ELLIS_ASSERT_EQ(en.as_map().length(), 0);
  ELLIS_ASSERT_TRUE(en.as_map().is_empty());
}

static void pathtest()
{
  using namespace ellis;
  node a(type::ARRAY);
  a.as_mutable_array().append(4);
  a.as_mutable_array().append("hi");
  node r(type::MAP);
  r.as_mutable_map().insert("foo", a);
  ELLIS_ASSERT_EQ(r.at_path("{foo}[0]"), 4);
  ELLIS_ASSERT_EQ(r.at_path("{foo}[1]"), "hi");
  auto chk_fail = [&r](const char *path)
  {
    bool threw = false;
    try {
      r.at_path(path);
    } catch(const err &e) {
      threw = true;
    }
    ELLIS_ASSERT_TRUE(threw);
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
  ELLIS_ASSERT_EQ(r.at_path("{foo}[0]"), 5);
}

int main()
{
  logtest();
  asserttest();
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
