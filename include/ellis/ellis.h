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

#ifndef __cplusplus
extern "C" {
#endif


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
 * @param[in] node a node
 * @param[out] err an error
 */
void ellis_node_ref(ellis_node *node, ellis_err **err);

/**
 * Decreases an ellis_node refcount by 1, freeing it if the count reaches 0.
 *
 * @param[in] node a node
 * @param[out] err an error
 */
void ellis_node_deref(ellis_node *node, ellis_err **err);

/**
 * Allocates a new node, using the given allocator.
 *
 * @param[in] alloc an allocator
 * @param[out] err an error
 *
 * @return a new node
 */
ellis_node *ellis_node_alloc(ellis_allocator *alloc, ellis_err **err);

/**
 * Initializes an empty array.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a new, empty array, or NULL if the function fails
 */
ellis_node *ellis_node_init_array(ellis_node *node, ellis_err **err);

/**
 * Initializes a real with the given value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a new real with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_init_real(ellis_node *node, double real, ellis_err **err);

/**
 * Initializes a new int with the given value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a new int, or NULL if the function fails
 */
ellis_node *ellis_node_init_int(ellis_node *node, uint64_t i, ellis_err **err);

/**
 * Initializes a Nil.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a Nil, or NULL if the function fails
 */
ellis_node *ellis_node_init_nil(ellis_node *node, void, ellis_err **err);

/**
 * Initializes a string.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a string, or NULL if the function fails
 */
ellis_node *ellis_node_init_string(
    ellis_node *node,
    const char *s,
    ellis_err **err);

/* TODO: is this a good idea, at all? */
/**
 * Initializes a string node from an input string without copying the input
 * string. It is invalid to later use the input string, as the node takes
 * control of it. When the node is deallocated, the input string will be freed
 * using the allocator associated with the ellis node.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a string, or NULL if the function fails
 */
ellis_node *ellis_node_init_string_steal(
    ellis_node *node,
    char *s, ellis_err **err);

/**
 * Allocates and initializes an empty array, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new, empty array, or NULL if the function fails
 */
ellis_node *ellis_node_create_array(ellis_err **err);

/**
 * Allocates and initializes a bool, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new bool with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_bool(ellis_bool val, ellis_err **err);

/**
 * Allocates and initializes a real, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new real with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_real(double real, ellis_err **err);

/**
 * Allocates and initializes an int, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new int with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_int(uint64_t real, ellis_err **err);

/**
 * Allocates and initializes a Nil, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new Nil with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_nil(void, ellis_err **err);

/**
 * Allocates and initializes a string, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new string with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_string(const char *s, ellis_err **err);

/* TODO: is this a good idea, at all? */
/**
 * Allocates and initializes a string node from an input string without copying
 * the input string. It is invalid to later use the input string, as the node
 * takes control of it. When the node is deallocated, the input string will be
 * freed using the allocator associated with the ellis node.
 *
 * @param[out] err an error
 *
 * @return a new string with the given value, or NULL if the function fails
 */
ellis_node *ellis_node_create_string_steal(char *s, ellis_err **err);

/**
 * Allocates and initializes an empty map, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new, empty map, or NULL if the function fails
 */
ellis_node *ellis_node_create_map(void, ellis_err **err);

/**
 * Initializes an empty map.
 *
 * @param[out] err an error
 *
 * @return a new, empty map, or NULL if the function fails
 */
ellis_node *ellis_node_init_map(void, ellis_err **err);

/**
 * Initializes a node from the contents of another node, performing a deep copy
 * such that the other node can be safely freed and the new node can continue to
 * be used.
 *
 * @param[in] node the node to initialize
 * @param[in] other the node from which to copy
 * @param[out] err an error
 *
 * @return node the node that was initialized
 */
ellis_node *ellis_node_init_copy(
    ellis_node *node,
    const ellis_node *other,
    ellis_err **err);

/* TODO: how should this work? does it make sense in C? */
/* TODO: doxygen */
ellis_node *ellis_node_init_steal(
    ellis_node *node,
    ellis_node *other,
    ellis_err **err);

/**
 * Deinitialize a node, nullifying the contents such that it is invalid to later
 * use the node in any way other than to call init or dealloc on it.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return the passed-in node
 */
ellis_node *ellis_node_deinit(ellis_node *node, ellis_err **err);

/**
 * Deallocates a node, freeing the memory to which it is associated. This
 * function is the reverse of the alloc function.
 *
 * @param[in] node a node
 * @param[out] err an error
 */
void ellis_node_dealloc(ellis_node *node, ellis_err **err);

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
 * @param[out] err an error
 *
 * @return the length of the array, in number of items.
 */
size_t ellis_array_length(const ellis_array *array, ellis_err **err);

/**
 * Gets the given index in the given array, increasing the ellis_node's refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index the index to get
 * @param[out] err an error
 *
 * @return the ellis_node at the given index, or NULL if there is none
 */
ellis_node *ellis_array_get(ellis_array *array, size_t index, ellis_err **err);

/**
 * Sets the given index in the given array to the given node.
 *
 * @param[in] array an array
 * @param[in] index the index to set
 * @param[in] node the node to set the index to
 * @param[out] err an error
 *
 * @return 0 if successful, -1 if not
 */
int ellis_array_set(
    ellis_array *array,
    size_t index,
    ellis_node *node,
    ellis_err **err);

/**
 * Sets the given index in the given array to the given node while releasing the
 * caller's reference to the node.
 *
 * @param[in] array an array
 * @param[in] index the index to set
 * @param[in] node the node to set the index to
 * @param[out] err an error
 *
 * @return 0 if successful, -1 if not
 */
int ellis_array_set_steal(
    ellis_array *array,
    size_t index,
    ellis_node *node,
    ellis_err **err);

/**
 * Appends the given node to the end of the given array.
 *
 * @param[in] array an array
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_append(ellis_array *array, ellis_node *node, ellis_err **err);

/**
 * Appends the given node to the end of the given array, releasing the caller's
 * reference to the node.
 *
 * @param[in] array an array
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_append_steal(
    ellis_array *array,
    ellis_node *node,
    ellis_err **err);

/**
 * Appends the given nodes to the end of the given array. Equivalent to calling
 * append many times.
 *
 * @param[in] array an array
 * @param[in] nodes a pointer to a list of nodes
 * @param[in] length the length (in nodes) of the nodes list
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_extend(
    ellis_array *array,
    ellis_node *nodes,
    size_t length,
    ellis_err **err);

/**
 * Appends the given nodes to the end of the given array, releasing the caller's
 * reference to the appended nodes. Equivalent to calling append_steal many
 * times.
 *
 * @param[in] array an array
 * @param[in] nodes a pointer to a list of nodes
 * @param[in] length the length (in nodes) of the nodes list
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_extend_steal(
    ellis_array *array,
    ellis_node *nodes,
    size_t length,
    ellis_err **err);

/**
 * Inserts the given node into the given array, after the item at the given
 * index.
 *
 * @param[in] array an array
 * @param[in] index an index into the array
 * @param[in] node a node to insert
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_insert(
    ellis_array *array,
    size_t index,
    ellis_node *node,
    ellis_err **err);

/**
 * Inserts the given node into the given array, after the item at the given
 * index, releasing the caller's reference to the node.
 *
 * @param[in] array an array
 * @param[in] index an index into the array
 * @param[in] node a node to insert
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_insert_steal(
    ellis_array *array,
    size_t index,
    ellis_node *node,
    ellis_err **err);

/**
 * Removes the node at the given index from the given array, decreasing the refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index an index to remove
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int ellis_array_remove(ellis_array *array, size_t index, ellis_err **err);

/**
 * Empties the contents of the given array, decreasing the refcount of each
 * contained item by 1.
 *
 * @param[in] array an array
 * @param[out] err an error
 */
void ellis_array_clear(ellis_array *array, ellis_err **err);

/**
 * A convenience macro to allow easy iteration of the given array. index will be
 * set to the current index in the iteration and value will be set to the
 * current node. Equivalent to calling the given code block inside a for loop
 * that uses ellis_array_get(array, index) and ellis_array_length(array).
 */
ellis_array_foreach(array, index, value, err)

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
 *
 * @param[out] err an error
 */
/* TODO: doxygen */
void ellis_binary_set(SOMETHING, ellis_err **err);

/* TODO: doxygen */
void ellis_binary_value(SOMETHING, ellis_err **err);

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
 * @param[in] node a node
 *
 * @return the node's value
 */
ellis_bool_val ellis_bool_value(const ellis_node *node, ellis_err **err);

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
 * @param[in] node a node
 *
 * @return the node's value
 */
uint64_t ellis_int_value(const ellis_node *node, ellis_err **err);

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
 * @param[in] node a node
 *
 * @return 1 if the node is Nil, 0 otherwise
 */
int ellis_is_nil(const ellis_node *node, ellis_err **err);

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
 * @param[in] node a node
 *
 * @return the node's value
 */
double ellis_real_value(const ellis_node *node, ellis_err **err);

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
 * @param[in] node a node
 *
 * @return the node's value
 */
const char *ellis_string_value(const ellis_node *node, ellis_err **err);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ELLIS_H_ */
