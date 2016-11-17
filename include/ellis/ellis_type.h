/*
 * @file type.h
 *
 * @brief Ellis public C header.
 *
 * This is the C header for Ellis, which implements the standalone Ellis data
 * interaction routines.
 */

#pragma once
#ifndef ELLIS_TYPE_H_
#define ELLIS_TYPE_H_

#ifndef __cplusplus
extern "C" {
#endif


/**
 * An enum containing all the Ellis data types.
 */
enum type {
  ELLIS_ARRAY,
  ELLIS_BINARY,
  ELLIS_BOOL,
  ELLIS_INT,
  ELLIS_MAP,
  ELLIS_NIL,
  ELLIS_REAL,
  ELLIS_STRING
};


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ELLIS_TYPE_H_ */
