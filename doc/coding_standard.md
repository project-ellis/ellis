# Coding Style Guide

## Spacing

* 80 chars, indent by 2 spaces, no tabs.
* No spaces on end of line.
* Source files end in a single newline, not 0, not 2.
* Double blank lines to separate function implementations, classes, or other large items.

## Naming Conventions

* Lower case with underscores in most cases.  (Slower to type, easier to read).
* Prefer short names, but long enough to be clear over realm used.
* Single letter variables OK in short blocks like loops.
* `g_foo` for global
* `m_foo` for member
* `s_foo` for static
* `t_foo` for thread-local

## Filename Conventions

* Filename matches class.
* Keep underscores as underscores in filename.
* All headers are meant to live in `ellis` subdir of `include`.
* Private headers: `ellis/private`.
* C headers: `.h` suffix.
* C++ headers: `.hpp` suffix.
* C implementation: `.c` suffix.
* C++ implementation: `.cpp` suffix.

## Bug Avoidance Practices

### General

* Compile with -Wall -Wextra -Werror.
* Use cppcheck.

### Headers

* Keep header files as *lightweight* as possible, with few dependencies.
* Headers include only what they need.
* When possible, *forward declare* types, and provide details in implementation.
* *C headers use unique prefixes* for all externally visible symbols.
* *C++ headers use namespaces* where appropriate instead of prefixes.
* C++ headers use fully namespace-qualified for external objects.
* Make prototypes *const correct* whenever possible.
* Make prototypes *resilient against accidental transposition* of arguments.

### Implementation

* C layer is a thin wrapper over C++ implementation.
* Use *RAII* whenever possible (`unique_ptr`, `unique_lock`, etc).
* Use *early return* whenever possible to take care of special cases.
* Implementations all import `<ellis/private/using.hpp>` for convenience.
* C++ headers do not use `using.hpp` nor pollute namespace with `using`.
* Keep contents of `using.hpp` short--carefully chosen set of ubiquitous types.
* *Always use braces* for if/for/etc even for single statement body.
* Use *static* file-scope variables and functions whenever possible.
* Do not leave code disabled except very temporarily.  Use `#if 0`, not
  comments, to disable.
* Don't casually mix signed and unsigned.
* Use *unsigned types for bit manipulation*.
* Use *extra parentheses* when unclear order of evaluation raises questions.
* Macros are allowed, but always *prefer inline*, or even a template if clean.
* *One variable declaration per line*, near where they are first used.
* Mark unused variables with UNUSED.

## Portability and Independence Practices

* Local versions of common defs in `ellis/private/defs.h`.
* User-specifiable allocation, logging, and fatal error callbacks.
* Prefer fixed width types when cross-platform behavior matters.
* Do not assume either way regarding endianness.

## C++ Header

In terms of the rule of 5, we make explicit in the header, either by declaring
that we will implement the function, or specifying "= delete" or "= default".

```
#pragma once
#ifndef ELLIS_NODE_HPP_
#define ELLIS_NODE_HPP_

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

#endif  // ELLIS_NODE_HPP_
```

## C Header

Only for public interfaces to be accessed from C.

```
#pragma once
#ifndef ELLIS_NODE_H_
#define ELLIS_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ellis_node ellis_node;

ellis_node *ellis_node_alloc(ellis_allocator *alloc);
ellis_node *ellis_node_init(ellis_node *node, const char *name);
ellis_node *ellis_node_create(const char *name);
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

#endif  // ELLIS_NODE_H_
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
