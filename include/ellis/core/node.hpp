/*
 * @file node.hpp
 *
 * @brief Ellis node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_NODE_HPP_
#define ELLIS_NODE_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/type.hpp>
#include <string>


namespace ellis {


/* Forward declaration. */
class array_node;
class binary_node;
class map_node;
struct payload;


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
    payload *m_pay;
    pad_t m_pad;
  };
  unsigned int m_type;

  /* Private methods--see implementation for description. */
  void _zap_contents(type t);
  void _grab_contents(const node &other);
  void _release_contents();
  void _prep_for_write();
  const bool        & _as_bool() const;
  const int64_t     & _as_int64() const;
  const double      & _as_double() const;
  const std::string & _as_u8str() const;
  const array_node  & _as_array() const;
  const map_node    & _as_map() const;
  const binary_node & _as_binary() const;
  bool        & _as_mutable_bool();
  int64_t     & _as_mutable_int64();
  double      & _as_mutable_double();
  std::string & _as_mutable_u8str();
  array_node  & _as_mutable_array();
  map_node    & _as_mutable_map();
  binary_node & _as_mutable_binary();

 public:

  /*   ____                _                   _
   *  / ___|___  _ __  ___| |_ _ __ _   _  ___| |_ ___  _ __ ___
   * | |   / _ \| '_ \/ __| __| '__| | | |/ __| __/ _ \| '__/ __|
   * | |__| (_) | | | \__ \ |_| |  | |_| | (__| || (_) | |  \__ \
   *  \____\___/|_| |_|___/\__|_|   \__,_|\___|\__\___/|_|  |___/
   *
   */

  /** Construct an ARRAY, MAP, or NIL node. */
  node(type);

  /** Construct a BINARY node. */
  node(const byte *mem, size_t bytes);

  /** Construct a BOOL node. */
  node(bool);

  /** Construct an INT64 node. */
  node(int);

  /** Construct an INT64 node. */
  node(int64_t);

  /** Construct an INT64 node. */
  node(unsigned int);

  /** Construct a DOUBLE node. */
  node(double);

  /** Construct a U8STR node. */
  node(const std::string&);

  /** Construct a U8STR node. */
  node(const char *);

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

  /** TODO: is this supposed to be related to _release_contents() and if
   * so why public?  Unimplemented.  Remove unless there is a purpose. */
  void release();

  ~node();


  /** Swap the contents of this node and the other. */
  void swap(node &other);

  /** Do a deep copy of the other node--new objects, new reference counter,
   * no sharing.
   *
   * The "other" node is allowed to be this very node; copy-on-write uses
   * this scenario to move self away from the original copy.
   */
  void deep_copy(const node &other);


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

  /** Assignment operators from primitive types.
   *
   * Any prior contents of this node are lost (refcount decremented).
   */
  node& operator=(bool o);
  node& operator=(double o);
  node& operator=(int o);
  node& operator=(unsigned int o);
  node& operator=(int64_t o);
  node& operator=(const char *o);
  node& operator=(const std::string &o);


  /** Arithmetic operators for arithmetic types.
   *
   * Arithmetic operations are valid only between the same types, so doing
   * arithmetic between an int64 and a double is not allowed.
   */
  node operator+=(const node &);
  node operator-=(const node &);
  node operator*=(const node &);
  node operator/=(const node &);

  node operator+=(int);
  node operator-=(int);
  node operator*=(int);
  node operator/=(int);

  node operator+=(unsigned int);
  node operator-=(unsigned int);
  node operator*=(unsigned int);
  node operator/=(unsigned int);

  node operator+=(int64_t);
  node operator-=(int64_t);
  node operator*=(int64_t);
  node operator/=(int64_t);

  node operator+=(double);
  node operator-=(double);
  node operator*=(double);
  node operator/=(double);

  /** Comparison operators for another node.
   *
   * Equality is defined as one might expect--types same, simple values same,
   * array sizes and elements same, map keys and values same.
   *
   * Note that we define the not equal operator, but we do not define
   * lesser/greater operators since there is not a clear scheme for ordering
   * at this time.
   */
  bool operator==(const node &) const;
  bool operator!=(const node &o) const;

  /** Comparison operators for primitive types.
   *
   * This implicitly verifies the type, e.g.:
   *
   *   mynode == 4.5
   *
   * Is equivalent to:
   *
   *   mynode.is_type(type::DOUBLE) && mynode.as_double() == 4.5
   *
   */
  bool operator==(bool o) const;
  bool operator==(double o) const;
  bool operator==(int o) const;
  bool operator==(unsigned int o) const;
  bool operator==(int64_t o) const;
  bool operator==(const char *o) const;
  bool operator==(const std::string &o) const;

  /** Inequality operators for primitive types.
   *
   * Same as (not (a==b)).
   */
  bool operator!=(bool o) const;
  bool operator!=(double o) const;
  bool operator!=(int o) const;
  bool operator!=(unsigned int o) const;
  bool operator!=(int64_t o) const;
  bool operator!=(const char *o) const;
  bool operator!=(const std::string &o) const;

  /** Typecasting operators for primitive types.
   *
   * These are convenience wrappers.  For example:
   *
   *   (bool)mynode
   *
   * is equivalent to:
   *
   *   mynode.as_bool()
   *
   * This means that an exception can be thrown if the type is incorrect,
   * as is the case with the as_xxx() converter functions.
   */
  operator bool() const;
  operator int() const;
  operator unsigned int() const;
  operator int64_t() const;
  operator double() const;
  explicit operator std::string() const;
  explicit operator const char *() const;


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


  /** Get contents as an bool.
   *
   * Will throw WRONG_TYPE error if type is not convertible.
   */
  const bool & as_bool() const;

  /** Get contents as an int64_t.
   *
   * Will throw WRONG_TYPE error if type is not convertible.
   */
  const int64_t & as_int64() const;

  /** Get contents as a double.
   *
   * Will throw WRONG_TYPE error if type is not convertible.
   */
  const double & as_double() const;

  /** Provide access to UTF-8 string contents.
   *
   * Will throw WRONG_TYPE error if type is not U8STR.
   */
  const std::string & as_u8str() const;
  const char * as_u8cstr() const;

  /** Provide access to array functionality.
   *
   * See array_node.hpp for more information.
   *
   * Will throw WRONG_TYPE error if type is not ARRAY.
   */
  const array_node & as_array() const;

  /** Provide access to map functionality.
   *
   * See map_node.hpp for more information.
   *
   * Will throw WRONG_TYPE error if type is not MAP.
   */
  const map_node & as_map() const;

  /** Provide access to binary blob contents.
   *
   * See binary_node.hpp for more information.
   *
   * Will throw WRONG_TYPE error if type is not BINARY.
   */
  const binary_node & as_binary() const;

  /** Get value from tree at given path (e.g. "{log}{handlers}[0]{sync}").
   */
  const node & at_path(const std::string &path) const;

  /** Mutable access to contents.
   *
   * Similar to immutable versions above, including type exceptions,
   * but the return value can be modified and the node will be updated.
   *
   * NOTE: since we don't have an easy hook for when you change the given
   * double pointer/reference, the copy-on-write logic is initiated as soon
   * as you request mutable access, even if you don't actually change it.
   */
  bool        & as_mutable_bool();
  int64_t     & as_mutable_int64();
  double      & as_mutable_double();
  std::string & as_mutable_u8str();
  array_node  & as_mutable_array();
  map_node    & as_mutable_map();
  binary_node & as_mutable_binary();
  node & at_path_mutable(const std::string &path);

  friend class array_node;
  friend class binary_node;
  friend class map_node;
};


/*
 *   ____                                 _
 *  / ___|___  _ __ ___  _ __   __ _ _ __(_)___  ___  _ __
 * | |   / _ \| '_ ` _ \| '_ \ / _` | '__| / __|/ _ \| '_ \
 * | |__| (_) | | | | | | |_) | (_| | |  | \__ \ (_) | | | |
 *  \____\___/|_| |_| |_| .__/ \__,_|_|  |_|___/\___/|_| |_|
 *                      |_|
 *   ___                       _
 *  / _ \ _ __   ___ _ __ __ _| |_ ___  _ __ ___
 * | | | | '_ \ / _ \ '__/ _` | __/ _ \| '__/ __|
 * | |_| | |_) |  __/ | | (_| | || (_) | |  \__ \
 *  \___/| .__/ \___|_|  \__,_|\__\___/|_|  |___/
 *       |_|
 */

bool operator<(const node &, const node &);
bool operator<=(const node &, const node &);
bool operator>(const node &, const node &);
bool operator>=(const node &, const node &);

bool operator<(const node &, int);
bool operator<=(const node &, int);
bool operator>(const node &, int);
bool operator>=(const node &, int);

bool operator<(int, const node &);
bool operator<=(int, const node &);
bool operator>(int, const node &);
bool operator>=(int, const node &);

bool operator<(const node &, unsigned int);
bool operator<=(const node &, unsigned int);
bool operator>(const node &, unsigned int);
bool operator>=(const node &, unsigned int);

bool operator<(unsigned int, const node &);
bool operator<=(unsigned int, const node &);
bool operator>(unsigned int, const node &);
bool operator>=(unsigned int, const node &);

bool operator<(const node &, int64_t);
bool operator<=(const node &, int64_t);
bool operator>(const node &, int64_t);
bool operator>=(const node &, int64_t);

bool operator<(int64_t, const node &);
bool operator<=(int64_t, const node &);
bool operator>(int64_t, const node &);
bool operator>=(int64_t, const node &);

bool operator<(const node &, double);
bool operator<=(const node &, double);
bool operator>(const node &, double);
bool operator>=(const node &, double);

bool operator<(double, const node &);
bool operator<=(double, const node &);
bool operator>(double, const node &);
bool operator>=(double, const node &);

/*
 *     _         _ _   _                    _   _
 *    / \   _ __(_) |_| |__  _ __ ___   ___| |_(_) ___
 *   / _ \ | '__| | __| '_ \| '_ ` _ \ / _ \ __| |/ __|
 *  / ___ \| |  | | |_| | | | | | | | |  __/ |_| | (__
 * /_/   \_\_|  |_|\__|_| |_|_| |_| |_|\___|\__|_|\___|
 *
 *   ___                       _
 *  / _ \ _ __   ___ _ __ __ _| |_ ___  _ __ ___
 * | | | | '_ \ / _ \ '__/ _` | __/ _ \| '__/ __|
 * | |_| | |_) |  __/ | | (_| | || (_) | |  \__ \
 *  \___/| .__/ \___|_|  \__,_|\__\___/|_|  |___/
 *       |_|
 *
 */
node operator+(const node &, const node &);
node operator-(const node &, const node &);
node operator*(const node &, const node &);
node operator/(const node &, const node &);

node operator/(const node &, int);
node operator+(const node &, int);
node operator-(const node &, int);
node operator*(const node &, int);

int operator/(int, const node &);
int operator+(int, const node &);
int operator-(int, const node &);
int operator*(int, const node &);

node operator/(const node &, unsigned int);
node operator+(const node &, unsigned int);
node operator-(const node &, unsigned int);
node operator*(const node &, unsigned int);

unsigned int operator/(unsigned int, const node &);
unsigned int operator+(unsigned int, const node &);
unsigned int operator-(unsigned int, const node &);
unsigned int operator*(unsigned int, const node &);

node operator/(const node &, int64_t);
node operator+(const node &, int64_t);
node operator-(const node &, int64_t);
node operator*(const node &, int64_t);

int64_t operator+(int64_t, const node &);
int64_t operator-(int64_t, const node &);
int64_t operator*(int64_t, const node &);
int64_t operator/(int64_t, const node &);

node operator/(const node &, double);
node operator+(const node &, double);
node operator-(const node &, double);
node operator*(const node &, double);

double operator/(double, const node &);
double operator+(double, const node &);
double operator-(double, const node &);
double operator*(double, const node &);


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


#endif  /* ELLIS_NODE_HPP_ */
