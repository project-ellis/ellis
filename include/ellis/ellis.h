/*
 * @file ellis.h
 *
 * @brief Ellis public C header.
 *
 * This is the C header for Ellis, which implements the standalone Ellis data
 * interaction routines.
 */

#pragma once
#ifndef ELLIS_H_
#define ELLIS_H_

/**
 *  ____        _          _____
 * |  _ \  __ _| |_ __ _  |_   _|   _ _ __   ___  ___
 * | | | |/ _` | __/ _` |   | || | | | '_ \ / _ \/ __|
 * | |_| | (_| | || (_| |   | || |_| | |_) |  __/\__ \
 * |____/ \__,_|\__\__,_|   |_| \__, | .__/ \___||___/
 *                              |___/|_|
 *
 */

/**
 * An enum containing all the Ellis data types.
 */
enum ellis_type {
  ELLIS_ARRAY,
  ELLIS_BINARY,
  ELLIS_BOOL,
  ELLIS_INT,
  ELLIS_MAP,
  ELLIS_NIL,
  ELLIS_REAL,
  ELLIS_STRING
};

/**
 * @brief A refcounted object that wraps an Ellis type; used for polymorphism.
 *
 * ellis_node is a refcounted object wrapping an Ellis type. Upon construction,
 * the refcount is set to 1, and the refcount is increased by 1 when the value
 * is stored (e.g. in an ellis_node or ellis_map). When the refcount reaches 0,
 * the ellis_node is freed. Note that ellis_nil is treated as a special case, as
 * it is a singleton that does not require memory allocation, so refcount
 * operations on it are no-ops.
 *
 */
typedef struct ellis_node ellis_node;

/**
 * Increase an ellis_node refcount by 1.
 *
 * @param[in] node an ellis_node
 */
void ellis_node_ref(ellis_node *node);

/**
 * Decrease an ellis_node refcount by 1, freeing it if the count reaches 0.
 *
 * @param[in] node an ellis_node
 */
void ellis_node_deref(ellis_node *node);

/**
 * An array.
 */
typedef struct ellis_array ellis_array;

/**
 * A binary blob.
 */
typedef struct ellis_binary ellis_binary;

/**
 * A boolean (true or false).
 */
typedef enum ellis_bool ellis_bool;

/**
 * An IEEE 754 signed double type.
 */
typedef struct ellis_double EllisReal;

/**
 * A 64-bit signed integer type.
 */
typedef struct ellis_int ellis_int;

/**
 * A key-value type.
 */
typedef struct ellis_map ellis_map;

/**
 * A singleton (nil, null, none) type.
 */
typedef struct ellis_nil ellis_nil;

/**
 * A UTF-8 string type.
 */
typedef struct ellis_string ellis_string;

/*
 * TODO: finish ellis_map
 */

/*
 *   ____                _                   _   _
 *  / ___|___  _ __  ___| |_ _ __ _   _  ___| |_(_) ___  _ __
 * | |   / _ \| '_ \/ __| __| '__| | | |/ __| __| |/ _ \| '_ \
 * | |__| (_) | | | \__ \ |_| |  | |_| | (__| |_| | (_) | | | |
 *  \____\___/|_| |_|___/\__|_|   \__,_|\___|\__|_|\___/|_| |_|
 *
 */

/*
 * TODO: [ TBD: return codes more specific than 0 vs. -1 ? ]
 */

/**
 * Constructs an empty array.
 *
 * @return a new, empty array, or NULL if the function fails
 */
ellis_array *ellis_array_make(void);

/**
 * Gets the length of the given array.
 *
 * @param[in] array an array
 *
 * @return the length of the array, in number of items.
 */
size_t ellis_array_length(ellis_array const *array);

/**
 * Gets the given index in the given array, increasing the ellis_node's refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index the index to get
 *
 * @return the ellis_node at the given index, or NULL if there is none
 */
ellis_node *ellis_array_get(ellis_array *array, size_t index);

/**
 * Sets the given index in the given array to the given node.
 *
 * @param[in] array an array
 * @param[in] index the index to set
 * @param[in] node the node to set the index to
 *
 * @return 0 if successful, -1 if not
 */
int ellis_array_set(ellis_array *array, size_t index, ellis_node *node);

/**
 * Sets the given index in the given array to the given node while releasing the
 * caller's reference to the node.
 *
 * @param[in] array an array
 * @param[in] index the index to set
 * @param[in] node the node to set the index to
 *
 * @return 0 if successful, -1 if not
 */
int ellis_arraySetSteal(ellis_array *array, size_t index, ellis_node *node);

/**
 * Appends the given node to the end of the given array.
 *
 * @param[in] array an array
 * @param[in] node a node
 *
 * @return 0 on success and -1 on error
 */
int ellis_arrayAppend(ellis_array *array, ellis_node *node);

/**
 * Appends the given node to the end of the given array, releasing the caller's
 * reference to the node.
 *
 * @param[in] array an array
 * @param[in] node a node
 *
 * @return 0 on success and -1 on error
 */
int ellis_arrayAppendSteal(ellis_array *array, ellis_node *node);

/**
 * Appends the given nodes to the end of the given array. Equivalent to calling
 * Append many times.
 *
 * @param[in] array an array
 * @param[in] nodes a pointer to a list of nodes
 * @param[in] length the length (in nodes) of the nodes list
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_extend(ellis_array *array, ellis_node *nodes, size_t length);
int ellis_array_extend_steal(
    ellis_array *array,
    ellis_node *nodes,
    size_t length);

/**
 * Inserts the given node into the given array, after the item at the given
 * index.
 *
 * @param[in] array an array
 * @param[in] index an index into the array
 * @param[in] node a node to insert
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_insert(ellis_array *array, size_t index, ellis_node *node);

/**
 * Inserts the given node into the given array, after the item at the given
 * index, releasing the caller's reference to the node.
 *
 * @param[in] array an array
 * @param[in] index an index into the array
 * @param[in] node a node to insert
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_insert_steal(
    ellis_array *array,
    size_t index,
    ellis_node *node);

/**
 * Removes the node at the given index from the given array, decreasing the refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index an index to remove
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_remove(ellis_array *array, size_t index);

/**
 * Empties the contents of the given array, decreasing the refcount of each
 * contained item by 1.
 *
 * @param[in] array an array
 */
void ellis_array_clear(ellis_array *array);

/**
 * A convenience macro to allow easy iteration of the given array. index will be
 * set to the current index in the iteration and value will be set to the
 * current node. Equivalent to calling the given code block inside a for loop
 * that uses ellis_array_get(array, index) and ellis_array_length(array).
 */
ellis_array_foreach(array, index, value)

/**
 * Constructs a bool.
 *
 * @return a new bool with the given value
 */
ellis_bool *ellis_bool_make(ellis_bool val);

/* TODO: pass in an ellis_node, don't have ellis_array type */
/* TODO: add get value functions for everything */
/* TODO: add an error type */
/* TODO: add init/alloc split */

/**
 * Constructs a real with the given value.
 *
 * @return a new real
 */
ellis_double *ellis_real_make(double val);

/**
 * Constructs a new int with the given value.
 *
 * @return a new int
 */
ellis_int *ellis_int_make(int val);

[ MAPS: WORK IN PROGRESS ]

/**
 * Constructs a Nil.
 *
 * @return a Nil
 */
ellis_nil *ellis_nil_make(void);

/**
 * Constructs a string.
 *
 * @return a string
 */
ellis_string *ellis_string_make(char const *val);

#endif /* ELLIS_H_ */
