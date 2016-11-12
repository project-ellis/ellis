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
  ELLIS_DOUBLE,
  ELLIS_INT,
  ELLIS_MAP,
  ELLIS_NIL,
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
typedef struct EllisBool EllisBool;

/**
 * An IEEE 754 signed double type.
 */
typedef struct EllisDouble EllisDouble;

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
 * TODO: finish doxygen
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
 * Constructs an empty EllisArray.
 *
 * @return a new array
 */
EllisArray *EllisArrayMake(void);

/*
 * Returns the length of the EllisArray (in number of EllisNode items).
 */
size_t EllisArrayLength(EllisArray const *array);

/* Gets the given index in the given EllisArray, increasing the EllisNode's refcount by 1. */
EllisNode *EllisArrayGet(EllisArray *array, size_t index);

/* Sets the given index in the given EllisArray to the given EllisNode. */
int EllisArraySet(EllisArray *array, size_t index, EllisNode *node);
int EllisArraySetSteal(EllisArray *array, size_t index, EllisNode *node);

/*
 * Appends the given EllisNode to the end of the given EllisArray. Returns 0 on success and -1 on error.
 */
int EllisArrayAppend(EllisArray *array, EllisNode *node);
int EllisArrayAppendSteal(EllisArray *array, EllisNode *node);

/*
 * Appends the given items to the end of the given EllisArray. Equivalent to calling Append many times. Returns 0 on success and -1 on error.
 */
int EllisArrayExtend(EllisArray *array, EllisNode *nodes, size_t length);
int EllisArrayExtendSteal(EllisArray *array, EllisNode *nodes, size_t length);

/*
 * Inserts the given EllisNode at the given index into the given EllisArray. Returns 0 on success and -1 on error.
 */
int EllisArrayInsert(EllisArray *array, size_t index, EllisNode *node);
int EllisArrayInsertSteal(EllisArray *array, size_t index, EllisNode *node);

/*
 * Removes the EllisNode at the given index from the given EllisArray, decreasing the refcount by 1. Returns 0 on success and -1 on error.
 */
Int EllisArrayRemove(EllisArray *array, EllisNode *node);

/*
 * Empties the contents of the given EllisArray, decreasing the refcount of each contained item by 1.
 */
void EllisArrayClear(EllisArray *array);

/* A convenience macro to allow easy iteration of the given EllisArray. index will be set to the current index in the iteration and value will be set to the current EllisNode *. Equivalent to calling the given code block inside a for loop that uses EllisArrayGet(array, index) and EllisArrayLength(array).
 */
EllisArrayForeach(array, index, value)

EllisBool *EllisBoolMake(int val);

EllisDouble *EllisDoubleMake(double val);

EllisInt *EllisIntMake(int val);

[ MAPS: WORK IN PROGRESS

/*
 * Constructs an empty EllisMap.
 */
EllisMap *EllisMapMake(void);

/*
 * Returns the length of the EllisMap (in number of EllisNode items).
 */
size_t EllisMapLength(EllisMap const *map);

EllisNil *EllisNilMake(void);

EllisString *EllisStringMake(char const *val);
EllisString *EllisStringSteal(char *val);

#endif /* ELLIS_H_ */
