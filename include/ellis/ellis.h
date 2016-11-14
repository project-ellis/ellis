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
enum EllisType {
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
 * EllisNode is a refcounted object wrapping an Ellis type.. Upon construction,
 * the refcount is set to 1, and the refcount is increased by 1 when the value
 * is stored (e.g. in an EllisNode or EllisMap). When the refcount reaches 0,
 * the EllisNode is freed. Note that EllisNil is treated as a special case, as
 * it is a singleton that does not require memory allocation, so refcount
 * operations on it are no-ops.
 *
 */
typedef struct EllisNode EllisNode;

/**
 * Increase an EllisNode refcount by 1.
 *
 * @param[in] node an EllisNode
 */
void EllisNodeRef(EllisNode *node);

/**
 * Decrease an EllisNode refcount by 1, freeing it if the count reaches 0.
 *
 * @param[in] node an EllisNode
 */
void EllisNodeDeref(EllisNode *node);

/**
 * An array.
 */
typedef struct EllisArray EllisArray;

/**
 * A binary blob.
 */
typedef struct EllisBinary EllisBinary;

/**
 * A boolean (true or false).
 */
typedef enum EllisBool EllisBool;

/**
 * An IEEE 754 signed double type.
 */
typedef struct EllisDouble EllisReal;

/**
 * A 64-bit signed integer type.
 */
typedef struct EllisInt EllisInt;

/**
 * A key-value type.
 */
typedef struct EllisMap EllisMap;

/**
 * A singleton (nil, null, none) type.
 */
typedef struct EllisNil EllisNil;

/**
 * A UTF-8 string type.
 */
typedef struct EllisString EllisString;

/*
 * TODO: finish EllisMap
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
EllisArray *EllisArrayMake(void);

/**
 * Gets the length of the given array.
 *
 * @param[in] array an array
 *
 * @return the length of the array, in number of items.
 */
size_t EllisArrayLength(EllisArray const *array);

/**
 * Gets the given index in the given array, increasing the EllisNode's refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index the index to get
 *
 * @return the EllisNode at the given index, or NULL if there is none
 */
EllisNode *EllisArrayGet(EllisArray *array, size_t index);

/**
 * Sets the given index in the given array to the given node.
 *
 * @param[in] array an array
 * @param[in] index the index to set
 * @param[in] node the node to set the index to
 *
 * @return 0 if successful, -1 if not
 */
int EllisArraySet(EllisArray *array, size_t index, EllisNode *node);

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
int EllisArraySetSteal(EllisArray *array, size_t index, EllisNode *node);

/**
 * Appends the given node to the end of the given array.
 *
 * @param[in] array an array
 * @param[in] node a node
 *
 * @return 0 on success and -1 on error
 */
int EllisArrayAppend(EllisArray *array, EllisNode *node);

/**
 * Appends the given node to the end of the given array, releasing the caller's
 * reference to the node.
 *
 * @param[in] array an array
 * @param[in] node a node
 *
 * @return 0 on success and -1 on error
 */
int EllisArrayAppendSteal(EllisArray *array, EllisNode *node);

/**
 * Appends the given mnodes to the end of the given array. Equivalent to calling
 * Append many times.
 *
 * @param[in] array an array
 * @param[in] nodes a pointer to a list of nodes
 * @param[in] length the length (in nodes) of the nodes list
 *
 * @return 0 on success and -1 on error
 */
int EllisArrayExtend(EllisArray *array, EllisNode *nodes, size_t length);
int EllisArrayExtendSteal(EllisArray *array, EllisNode *nodes, size_t length);

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
int EllisArrayInsert(EllisArray *array, size_t index, EllisNode *node);

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
int EllisArrayInsertSteal(EllisArray *array, size_t index, EllisNode *node);

/**
 * Removes the node at the given index from the given array, decreasing the refcount by 1.
 *
 * @param[in] array an array
 * @param[in] index an index to remove
 *
 * @return 0 on success and -1 on error
 */
int EllisArrayRemove(EllisArray *array, size_t index);

/**
 * Empties the contents of the given array, decreasing the refcount of each
 * contained item by 1.
 *
 * @param[in] array an array
 */
void EllisArrayClear(EllisArray *array);

/**
 * A convenience macro to allow easy iteration of the given array. index will be
 * set to the current index in the iteration and value will be set to the
 * current node. Equivalent to calling the given code block inside a for loop
 * that uses EllisArrayGet(array, index) and EllisArrayLength(array).
 */
EllisArrayForeach(array, index, value)

/**
 * Constructs a bool.
 *
 * @return a new bool with the given value
 */
EllisBool *EllisBoolMake(EllisBool val);

/* TODO: lowercase underscores */
/* TODO: add get value functions for everything */
/* TODO: add an error type */
/* TODO: pass in an EllisNode, don't have EllisArray type */
/* TODO: add init/alloc split */

/**
 * Constructs a real with the given value.
 *
 * @return a new real
 */
EllisDouble *EllisRealMake(double val);

/**
 * Constructs a new int with the given value.
 *
 * @return a new int
 */
EllisInt *EllisIntMake(int val);

[ MAPS: WORK IN PROGRESS ]

/**
 * Constructs a Nil.
 *
 * @return a Nil
 */
EllisNil *EllisNilMake(void);

/**
 * Constructs a string.
 *
 * @return a string
 */
EllisString *EllisStringMake(char const *val);

#endif /* ELLIS_H_ */
