# Coding Style Guide

## Spacing

80 chars, indent by 2 spaces, no tabs.

Source files end in a single newline, not 0, not 2.

Double blank lines to separate function implementations, classes, or other
large items.

## C++ Header

In terms of the rule of 5, we make explicit in the header, either by declaring
that we will implement, specifying "= delete" or "= default".

```
#pragma once
#ifndef ELLIS_FOO_HPP_
#define ELLIS_FOO_HPP_

namespace ellis {

class node {
  std::string m_myname;

public:
  node(const std::string &name);
  node(const node &lunch) = delete;
  node(node &&other);
  node& operator=(const node &other) = delete;
  node& operator=(node &&lunch);
  ~node();

  int do_innocuous_thing() const;
  int do_mutating_thing();
  void graft_node(const std::string &path, node &&lunch);
  void big_bad(
    const std::string &a,
    const std::string &b,
    const std::string &c,
    const std::string &d);
};

}  // namespace ellis

#endif  // ELLIS_FOO_HPP_
```

## C Header

Only for public interfaces to be accessed from C.

```
#pragma once
#ifndef ELLIS_FOO_H_
#define ELLIS_FOO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ellis_node ellis_node;

ellis_node *ellis_node_alloc(ellis_allocator *alloc);
ellis_node *ellis_node_init(ellis_node *node, const char *name);
ellis_node *ellis_node_init_copy(ellis_node *node, const ellis_node *other);
ellis_node *ellis_node_init_move(ellis_node *node, ellis_node *other);

ellis_node *ellis_node_deinit(ellis_node *node);
void ellis_node_dealloc(ellis_node *node);

int ellis_node_do_innocuous_thing(void);
int ellis_node_do_mutating_thing(void);
void ellis_node_graft_node(const char *path, ellis_node *lunch);

#ifdef __cplusplus
}
#endif

#endif  // ELLIS_FOO_H_
```

## C++ Implementation

Public header files live in include/ellis.
Private header files live in include/ellis/private.

Includes are separated into categories:

* My own ellis header file, to catch mismatches.
* All other headers, sorted alphabetically.

Keep functions short (prefer under 30 lines), and avoid deep nesting when
possible.  If you have to think about exceeding 80 chars, you are probably
doing it wrong.

### Example

```
#include <ellis/ellis_node.hpp>

#include <ellis/private/builtins.hpp>
#include <ellis/private/using.hpp>
#include <stdio.h>
#include <vector>

namespace ellis {

void *g_initialized = nullptr;

static void
helper_func(const string &s) {
}

void ellis_node::ellis_node(
  std::string &name) :
    m_myname(name)
{
}

void ellis_node::big_bad(
  const std::string &a,
  const std::string &b,
  const std::string &c,
  const std::string &d)
{
  if (something_wrong) {
    return;
  }

  for (const auto &it : m_myname) {
    if (it) {
      blah(it);
    }
  }
}

}  // namespace ellis
```
