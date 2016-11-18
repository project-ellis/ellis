/*
 * @file node.hpp
 *
 * @brief Ellis node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_NODE_HPP_
#define ELLIS_NODE_HPP_

#include <ellis/type.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vector>

namespace ellis {


/* Forward declaration. */
class array_node;
class map_node;
struct prize_blk;


/* TODO: steal/adapt doxygen documentation from the C headers */
/* TODO: name the variables in the function prototypes */


/** node is an in-memory generic hierarchical node that can represent a
 * map, array, integer, or string, etc, corresponding to a hierarchical object
 * expressed in a variety of formats, such as XML, JSON, BSON, or msgpack.
 *
 * It is intended as a convenient interoperable in-memory object (representing
 * unpacked data received from network or disk in some serialized form), and
 * for general purpose conveyance between functions in a program after having
 * been received.  For example, a server might receive an node using one
 * serialization format, modify the node, store it into a queue, and
 * then send it elsewhere in a completely different format.
 *
 * This object has ref-counted copy-on-write semantics--if you make a copy,
 * then the reference count of the source will be incremented, but if you
 * then modify the copy, then an actual copy will occur at the given node,
 * with new reference count equal to one, and the original reference count
 * will decrease.
 */
class node {

  using pad_t = char[8];  /* pad covers union--checked via static_assert */
  union {
    bool m_boo;
    double m_dbl;
    int64_t m_int;
    std::string m_str;
    prize_blk *m_blk;
    pad_t m_pad;
  };
  unsigned m_type;

  bool _is_refcounted();
  void _zap_contents(type t);
  void _grab_contents(const node &other);
  void _release_contents();

 private:
  /** Get contents as an int64_t.
   *
   * Will not check type.
   */
  int64_t _as_int64() const;

  /** Get contents as a double.
   *
   * Will not check type.
   */
  double _as_double() const;

  /** Provide access to UTF-8 string contents.
   *
   * Will not check type.
   */
  const std::string & _as_u8str() const;

  /** Provide access to array functionality.
   *
   * Will not check type.
   */
  array_node & _as_array();
  const array_node & _as_array() const;

  /** Provide access to map functionality.
   *
   * Will not check type.
   */
  map_node & _as_map();
  const map_node & _as_map() const;

  /** Provide access to binary blob contents.
   *
   * Fills in the size parameter with the actual size of binary data.
   * Will not check type.
   */
  uint8_t* _as_binary(size_t *size);
  const uint8_t* _as_binary(size_t *size) const;


 public:

  /*   ____                _                   _
   *  / ___|___  _ __  ___| |_ _ __ _   _  ___| |_ ___  _ __ ___
   * | |   / _ \| '_ \/ __| __| '__| | | |/ __| __/ _ \| '__/ __|
   * | |__| (_) | | | \__ \ |_| |  | |_| | (__| || (_) | |  \__ \
   *  \____\___/|_| |_|___/\__|_|   \__,_|\___|\__\___/|_|  |___/
   *
   */

  // TODO: experiment re safety w/o explicit constructors

  /** Construct an ARRAY, MAP, or NIL node. */
  explicit node(type);

  /** Construct a BINARY node. */
  explicit node(const uint8_t *mem, size_t bytes);

  /** Construct a BOOL node. */
  explicit node(bool);

  /** Construct a U8STR node. */
  explicit node(const std::string&);

  /** Construct a U8STR node with move sementics. */
  explicit node(std::string&);

  /** Construct an INT64 node. */
  explicit node(int64_t);

  /** Construct a DOUBLE node. */
  explicit node(double);

  /** Copy constructor.
   *
   * Shares contents by incrementing ref count.  Contents are subject to
   * copy on write, until refcount goes down to 1 again. */
  node(const node& other);

  /** Move constructor.
   *
   * Steals contents, without changing ref count.  Other no longer has a
   * reference count, and will not affect the contents when deleted. */
  node(node&& other);


  /*  ____            _                   _
   * |  _ \  ___  ___| |_ _ __ _   _  ___| |_ ___  _ __
   * | | | |/ _ \/ __| __| '__| | | |/ __| __/ _ \| '__|
   * | |_| |  __/\__ \ |_| |  | |_| | (__| || (_) | |
   * |____/ \___||___/\__|_|   \__,_|\___|\__\___/|_|
   *
   */

  void release();
  ~node();


  void swap(node &other);


  /*   ___                       _
   *  / _ \ _ __   ___ _ __ __ _| |_ ___  _ __ ___
   * | | | | '_ \ / _ \ '__/ _` | __/ _ \| '__/ __|
   * | |_| | |_) |  __/ | | (_| | || (_) | |  \__ \
   *  \___/| .__/ \___|_|  \__,_|\__\___/|_|  |___/
   *       |_|
   */

  /** Copy assignment operator.
   *
   * Shares the RHS's contents via COW, as in copy constructor.
   *
   * Any prior contents of this node are lost (refcount decremented).
   */
  node& operator=(const node& rhs);

  /** Move assignment operator.
   *
   * Steals the RHS's contents, as in move constructor.
   *
   * Any prior contents of this node are lost (refcount decremented).
   */
  node& operator=(node&& rhs);

  /** Equality operator.
   *
   * Equality is defined as one might expect--types same, simple values same,
   * array sizes and elements same, map keys and values same.
   *
   * Note that we also define not equal operator, but we do not define
   * lesser/greater operators since there is not a clear scheme for ordering
   * at this time.
   */
  bool operator==(const node &) const;

  /** Inequality operator.
   *
   * Same as (not (a==b)).
   */
  bool operator!=(const node &o) const { return not (*this == o); }


  /*  _____
   * |_   _|   _ _ __   ___
   *   | || | | | '_ \ / _ \
   *   | || |_| | |_) |  __/
   *   |_| \__, | .__/ \___|
   *       |___/|_|
   */


  /** Check whether this node is of type t. */
  bool is_type(type t) const;

  /** Return the type of this node. */
  type get_type() const;


  /*   ____            _             _
   *  / ___|___  _ __ | |_ ___ _ __ | |_ ___
   * | |   / _ \| '_ \| __/ _ \ '_ \| __/ __|
   * | |__| (_) | | | | ||  __/ | | | |_\__ \
   *  \____\___/|_| |_|\__\___|_| |_|\__|___/
   *
   */


  /** Get contents as an int64_t.
   *
   * Will throw WRONG_TYPE error if type is not convertible.
   */
  explicit operator int64_t() const;
  int64_t as_int64() const;

  /** Get contents as a double.
   *
   * Will throw WRONG_TYPE error if type is not convertible.
   */
  explicit operator double() const;
  double as_double() const;

  /** Provide access to UTF-8 string contents.
   *
   * Will throw WRONG_TYPE error if type is not U8STR.
   */
  const std::string & as_u8str() const;

  /** Provide access to array functionality.
   *
   * Will throw WRONG_TYPE error if type is not ARRAY.
   */
  array_node & as_array();
  const array_node & as_array() const;

  /** Provide access to map functionality.
   *
   * Will throw WRONG_TYPE error if type is not MAP.
   */
  map_node & as_map();
  const map_node & as_map() const;

  /** Provide access to binary blob contents.
   *
   * Fills in the size parameter with the actual size of binary data.
   * Will throw WRONG_TYPE error if type is not BINARY.
   */
  uint8_t* as_binary(size_t *size);
  const uint8_t* as_binary(size_t *size) const;


  /** Get value from tree at given path (e.g. "{log}{handlers}[0]{sync}").
   *
   * If successful, will cause reference counts to increment for the relevant
   * subtree rooted at that path.
   */
  node & get_path(const std::string &path);
  const node & get_path(const std::string &path) const;

  friend class array_node;
  friend class map_node;
};


namespace prize_types {
  using arr_t = std::vector<node>;
  using map_t = std::unordered_map<std::string, node>;
  using bin_t = std::vector<uint8_t>;
  using refcount_t = unsigned;
}


struct prize_blk {
  prize_types::refcount_t m_refcount;
  union {
    prize_types::arr_t m_arr;
    prize_types::map_t m_map;
    prize_types::bin_t m_bin;
  };
};


/*  ___          _
 * |_ _|___  ___| |_ _ __ ___  __ _ _ __ ___
 *  | |/ _ \/ __| __| '__/ _ \/ _` | '_ ` _ \
 *  | | (_) \__ \ |_| | |  __/ (_| | | | | | |
 * |___\___/|___/\__|_|  \___|\__,_|_| |_| |_|
 *
 *   ___                       _
 *  / _ \ _ __   ___ _ __ __ _| |_ ___  _ __ ___
 * | | | | '_ \ / _ \ '__/ _` | __/ _ \| '__/ __|
 * | |_| | |_) |  __/ | | (_| | || (_) | |  \__ \
 *  \___/| .__/ \___|_|  \__,_|\__\___/|_|  |___/
 *       |_|
 */


std::ostream & operator<<(std::ostream & os, const node & v);
std::istream & operator>>(std::istream & is, node & v);


}  /* namespace ellis */


#endif /* ELLIS_NODE_HPP_ */
