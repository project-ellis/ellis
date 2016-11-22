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


/* TODO: add map */
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
enum type {
  ELLIS_ARRAY,
  ELLIS_BINARY,
  ELLIS_BOOL,
  ELLIS_DOUBLE,
  ELLIS_INT64,
  ELLIS_MAP,
  ELLIS_NIL,
  ELLIS_U8STR
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
 * node is a refcounted object wrapping an Ellis type. Upon construction,
 * the refcount is set to 1, and the refcount is increased by 1 when the value
 * is stored (e.g. in an node or map). When the refcount reaches 0,
 * the node is freed. Note that nil is treated as a special case, as
 * it is a singleton that does not require memory allocation, so refcount
 * operations on it are no-ops.
 *
 */
typedef struct node node;

/**
 * Increases an node refcount by 1.
 *
 * @param[in] node a node
 * @param[out] err an error
 */
void node_ref(node *node, err **err);

/**
 * Decreases an node refcount by 1, freeing it if the count reaches 0.
 *
 * @param[in] node a node
 * @param[out] err an error
 */
void node_deref(node *node, err **err);

/**
 * Allocates a new node, using the given allocator.
 *
 * @param[in] alloc an allocator
 * @param[out] err an error
 *
 * @return a new node
 */
node *node_alloc(allocator *alloc, err **err);

/**
 * Initializes an empty array.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a new, empty array, or NULL if the function fails
 */
node *node_init_array(node *node, err **err);

/**
 * Initializes a double with the given value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a new double with the given value, or NULL if the function fails
 */
node *node_init_double(node *node, double d, err **err);

/**
 * Initializes a new int with the given value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a new int, or NULL if the function fails
 */
node *node_init_int(node *node, int64_t i, err **err);

/**
 * Initializes a Nil.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a Nil, or NULL if the function fails
 */
node *node_init_nil(node *node, void, err **err);

/**
 * Initializes a u8str.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return a u8str, or NULL if the function fails
 */
node *node_init_u8str(
    node *node,
    const char *s,
    err **err);

/**
 * Allocates and initializes an empty array, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new, empty array, or NULL if the function fails
 */
node *node_create_array(err **err);

/**
 * Allocates and initializes a bool, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new bool with the given value, or NULL if the function fails
 */
node *node_create_bool(bool val, err **err);

/**
 * Allocates and initializes a double, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new double with the given value, or NULL if the function fails
 */
node *node_create_double(double d, err **err);

/**
 * Allocates and initializes an int, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new int with the given value, or NULL if the function fails
 */
node *node_create_int(int64_t i, err **err);

/**
 * Allocates and initializes a Nil, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new Nil with the given value, or NULL if the function fails
 */
node *node_create_nil(void, err **err);

/**
 * Allocates and initializes a u8str, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new u8str with the given value, or NULL if the function fails
 */
node *node_create_u8str(const char *s, err **err);

/**
 * Allocates and initializes an empty map, as a convenience.
 *
 * @param[out] err an error
 *
 * @return a new, empty map, or NULL if the function fails
 */
node *node_create_map(void, err **err);

/**
 * Initializes an empty map.
 *
 * @param[out] err an error
 *
 * @return a new, empty map, or NULL if the function fails
 */
node *node_init_map(void, err **err);

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
node *node_init_copy(
    node *node,
    const node *other,
    err **err);

/* TODO: doxygen */
node *node_init_move(
    node *node,
    node *other,
    err **err);

/**
 * Deinitialize a node, nullifying the contents such that it is invalid to later
 * use the node in any way other than to call init or dealloc on it.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return the passed-in node
 */
node *node_deinit(node *node, err **err);

/**
 * Deallocates a node, freeing the memory to which it is associated. This
 * function is the reverse of the alloc function.
 *
 * @param[in] node a node
 * @param[out] err an error
 */
void node_dealloc(node *node, err **err);

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
size_t array_length(const array *array, err **err);

/**
 * Gets the given index in the given array, increasing the node's refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index the index to get
 * @param[out] err an error
 *
 * @return the node at the given index, or NULL if there is none
 */
node *array_get(array *array, size_t index, err **err);

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
int array_set(
    array *array,
    size_t index,
    node *node,
    err **err);

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
int array_set_move(
    array *array,
    size_t index,
    node *node,
    err **err);

/**
 * Appends the given node to the end of the given array.
 *
 * @param[in] array an array
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int array_append(array *array, node *node, err **err);

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
int array_append_move(
    array *array,
    node *node,
    err **err);

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
int array_extend(
    array *array,
    node *nodes,
    size_t length,
    err **err);

/**
 * Appends the given nodes to the end of the given array, releasing the caller's
 * reference to the appended nodes. Equivalent to calling append_move many
 * times.
 *
 * @param[in] array an array
 * @param[in] nodes a pointer to a list of nodes
 * @param[in] length the length (in nodes) of the nodes list
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int array_extend_move(
    array *array,
    node *nodes,
    size_t length,
    err **err);

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
int array_insert(
    array *array,
    size_t index,
    node *node,
    err **err);

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
int array_insert_move(
    array *array,
    size_t index,
    node *node,
    err **err);

/**
 * Removes the node at the given index from the given array, decreasing the refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index an index to remove
 * @param[out] err an error
 *
 * @return 0 on success and -1 on error
 */
int array_remove(array *array, size_t index, err **err);

/**
 * Empties the contents of the given array, decreasing the refcount of each
 * contained item by 1.
 *
 * @param[in] array an array
 * @param[out] err an error
 */
void array_clear(array *array, err **err);

/**
 * A convenience macro to allow easy iteration of the given array. index will be
 * set to the current index in the iteration and value will be set to the
 * current node. Equivalent to calling the given code block inside a for loop
 * that uses array_get(array, index) and array_length(array).
 */
array_foreach(array, index, value, err)

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
void binary_set(SOMETHING, err **err);

/* TODO: doxygen */
void binary_value(SOMETHING, err **err);

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
enum bool_val {
  ELLIS_FALSE = 0,
  ELLIS_TRUE = 1
};

/**
 * Gets the underlying bool value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return the node's value
 */
bool_val bool_value(const node *node, err **err);

/*
 *  ____              _     _
 * |  _ \  ___  _   _| |__ | | ___
 * | | | |/ _ \| | | | '_ \| |/ _ \
 * | |_| | (_) | |_| | |_) | |  __/
 * |____/ \___/ \__,_|_.__/|_|\___|
 *
 */

/**
 * Gets the underlying double value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return the node's value
 */
double double_value(const node *node, err **err);

/*
 *  ___       _    __   _  _
 * |_ _|_ __ | |_ / /_ | || |
 *  | || '_ \| __| '_ \| || |_
 *  | || | | | |_| (_) |__   _|
 * |___|_| |_|\__|\___/   |_|
 *
 */

/**
 * Gets the underlying int value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return the node's value
 */
int64_t int_value(const node *node, err **err);

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
 * @param[out] err an error
 *
 * @return 1 if the node is Nil, 0 otherwise
 */
int is_nil(const node *node, err **err);

/*
 *  _   _  ___      _
 * | | | |( _ ) ___| |_ _ __
 * | | | |/ _ \/ __| __| '__|
 * | |_| | (_) \__ \ |_| |
 *  \___/ \___/|___/\__|_|
 *
 */

/**
 * Gets the underlying u8str value.
 *
 * @param[in] node a node
 * @param[out] err an error
 *
 * @return the node's value
 */
const char *u8str_value(const node *node, err **err);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* ELLIS_H_ */
