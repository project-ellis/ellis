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

/* TODO: add an error type */
/* TODO: add ellis_map */
/* TODO: zero-copy interface */

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

/*
 *  ____        _          _ _  __      _   _
 * |  _ \  __ _| |_ __ _  | (_)/ _| ___| |_(_)_ __ ___   ___
 * | | | |/ _` | __/ _` | | | | |_ / _ \ __| | '_ ` _ \ / _ \
 * | |_| | (_| | || (_| | | | |  _|  __/ |_| | | | | | |  __/
 * |____/ \__,_|\__\__,_| |_|_|_|  \___|\__|_|_| |_| |_|\___|
 *
 */

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
 * Increases an ellis_node refcount by 1.
 *
 * @param[in] node an ellis_node
 */
void ellis_node_ref(ellis_node *node);

/**
 * Decreases an ellis_node refcount by 1, freeing it if the count reaches 0.
 *
 * @param[in] node an ellis_node
 */
void ellis_node_deref(ellis_node *node);

/**
 * Allocates a new node, using the given allocator.
 *
 * @param alloc an allocator
 *
 * @return a new node
 */
ellis_node *ellis_node_alloc(ellis_allocator *alloc);

/**
 * Initializes an empty array.
 *
 * @return a new, empty array, or NULL if the function fails
 */
ellis_node *ellis_node_init_array(void);

/**
 * Initializes a real with the given value.
 *
 * @return a new real with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_init_real(double real);

/**
 * Initializes a new int with the given value.
 *
 * @return a new int, or NULL if the function fails
 */
ellis_node *ellis_node_init_int(uint64_t i);

/**
 * Initializes a Nil.
 *
 * @return a Nil, or NULL if the function fails
 */
ellis_node *ellis_node_init_nil(void);

/**
 * Initializes a string.
 *
 * @return a string, or NULL if the function fails
 */
ellis_node *ellis_node_init_string(char const *s);

/* TODO: is this a good idea, at all? */
/**
 * Initializes a string node from an input string without copying the input
 * string. It is invalid to later use the input string, as the node takes
 * control of it. When the node is deallocated, the input string will be freed
 * using the allocator associated with the ellis node.
 *
 * @return a string, or NULL if the function fails
 */
ellis_node *ellis_node_init_string_steal(char *s);

/**
 * Allocates and initializes an empty array, as a convenience.
 *
 * @return a new, empty array, or NULL if the function fails
 */
ellis_node *ellis_node_create_array(void);

/**
 * Allocates and initializes a bool, as a convenience.
 *
 * @return a new bool with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_bool(ellis_bool val);

/**
 * Allocates and initializes a real, as a convenience.
 *
 * @return a new real with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_real(double real);

/**
 * Allocates and initializes an int, as a convenience.
 *
 * @return a new int with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_int(uint64_t real);

/**
 * Allocates and initializes a Nil, as a convenience.
 *
 * @return a new Nil with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_nil(void);

/**
 * Allocates and initializes a string, as a convenience.
 *
 * @return a new string with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_string(const char *s);

/* TODO: is this a good idea, at all? */
/**
 * Allocates and initializes a string node from an input string without copying
 * the input string. It is invalid to later use the input string, as the node
 * takes control of it. When the node is deallocated, the input string will be
 * freed using the allocator associated with the ellis node.
 *
 * @return a new string with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_string_steal(char *s);

/**
 * Allocates and initializes an empty map, as a convenience.
 *
 * @return a new, empty map, or NULL if the function fails
 */
ellis_node *ellis_node_create_map(void);

/**
 * Initializes an empty map.
 *
 * @return a new, empty map, or NULL if the function fails
 */
ellis_node *ellis_node_init_map(void);

/**
 * Initializes a node from the contents of another node, performing a deep copy
 * such that the other node can be safely freed and the new node can continue to
 * be used.
 *
 * @param node the node to initialize
 * @param other the node from which to copy
 *
 * @return node the node that was initialized
 */
ellis_node *ellis_node_init_copy(ellis_node *node, const ellis_node *other);

/* TODO: how should this work? does it make sense in C? */
/* TODO: doxygen */
ellis_node *ellis_node_init_steal(ellis_node *node, ellis_node *other);

/**
 * Deinitialize a node, nullifying the contents such that it is invalid to later
 * use the node in any way other than to call init or dealloc on it.
 *
 * @param node a node
 *
 * @return the passed-in node
 */
ellis_node *ellis_node_deinit(ellis_node *node);

/**
 * Deallocates a node, freeing the memory to which it is associated. This
 * function is the reverse of the alloc function.
 *
 * @param node a node
 */
void ellis_node_dealloc(ellis_node *node);

/*
 *     _
 *    / \   _ __ _ __ __ _ _   _
 *   / _ \ | '__| '__/ _` | | | |
 *  / ___ \| |  | | | (_| | |_| |
 * /_/   \_\_|  |_|  \__,_|\__, |
 *                         |___/
 */

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
int ellis_array_set_steal(ellis_array *array, size_t index, ellis_node *node);

/**
 * Appends the given node to the end of the given array.
 *
 * @param[in] array an array
 * @param[in] node a node
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_append(ellis_array *array, ellis_node *node);

/**
 * Appends the given node to the end of the given array, releasing the caller's
 * reference to the node.
 *
 * @param[in] array an array
 * @param[in] node a node
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_append_steal(ellis_array *array, ellis_node *node);

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

/*
 *  ____  _
 * | __ )(_)_ __   __ _ _ __ _   _
 * |  _ \| | '_ \ / _` | '__| | | |
 * | |_) | | | | | (_| | |  | |_| |
 * |____/|_|_| |_|\__,_|_|   \__, |
 *                           |___/
 *
 */

/* TODO: zero-copy interface */
/**
 * Set a binary object to the given contents.
 */
/* TODO: doxygen */
void ellis_binary_set(SOMETHING);

/* TODO: doxygen */
void ellis_binary_value(SOMETHING);

/*
 *  ____              _
 * | __ )  ___   ___ | |
 * |  _ \ / _ \ / _ \| |
 * | |_) | (_) | (_) | |
 * |____/ \___/ \___/|_|
 *
 */

/**
 * The underlying value of a bool.
 */
enum ellis_bool_val {
  ELLIS_FALSE = 0,
  ELLIS_TRUE = 1
};

/**
 * Gets the underlying bool value.
 *
 * @param node a node
 *
 * @return the node's value
 */
ellis_bool_val ellis_bool_value(const ellis_node *node);

/*
 *  ___       _
 * |_ _|_ __ | |_
 *  | || '_ \| __|
 *  | || | | | |_
 * |___|_| |_|\__|
 *
 */

/**
 * Gets the underlying int value.
 *
 * @param node a node
 *
 * @return the node's value
 */
uint64_t ellis_int_value(const ellis_node *node);

/*
 *  __  __
 * |  \/  | __ _ _ __
 * | |\/| |/ _` | '_ \
 * | |  | | (_| | |_) |
 * |_|  |_|\__,_| .__/
 *              |_|
 *
 */

/* TODO */

/*
 *  _   _ _ _
 * | \ | (_) |
 * |  \| | | |
 * | |\  | | |
 * |_| \_|_|_|
 *
 */

/**
 * Checks if a node is Nil.
 *
 * @param node a node
 *
 * @return 1 if the node is Nil, 0 otherwise
 */
int ellis_is_nil(const ellis_node *node);

/*
 *  ____            _
 * |  _ \ ___  __ _| |
 * | |_) / _ \/ _` | |
 * |  _ <  __/ (_| | |
 * |_| \_\___|\__,_|_|
 *
 */

/**
 * Gets the underlying double value.
 *
 * @param node a node
 *
 * @return the node's value
 */
double ellis_real_value(const ellis_node *node);

/*
 *  ____  _        _
 * / ___|| |_ _ __(_)_ __   __ _
 * \___ \| __| '__| | '_ \ / _` |
 *  ___) | |_| |  | | | | | (_| |
 * |____/ \__|_|  |_|_| |_|\__, |
 *                         |___/
 *
 */

/**
 * Gets the underlying string value.
 *
 * @param node a node
 *
 * @return the node's value
 */
const char *ellis_string_value(const ellis_node *node);

#endif /* ELLIS_H_ */
