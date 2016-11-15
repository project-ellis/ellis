/*
 * @file ellis_err.h
 *
 * @brief Ellis public C header.
 *
 * This is the C header for Ellis, which implements the standalone Ellis data
 * interaction routines.
 */

#pragma once
#ifndef ELLIS_ERR_H_
#define ELLIS_ERR_H_

#ifndef __cplusplus
extern "C" {
#endif


/**
 * @brief An object representing an error, including a code and message.
 *
 * When calling ellis functions, a pointer to an ellis_err pointer (initially
 * set to null) is provided to the ellis function for storing the address of
 * any error that has been generated in the course of the function.
 *
 * To ignore errors, the pointer to a pointer should be set to NULL.
 *
 * The user should call the error free function.
 */
typedef struct ellis_err ellis_err;

/**
 * Gets the error code associated with a given error. This code will be in the
 * form -errno, with errno values coming from either the standard errno.h or
 * ellis/errno.h.
 *
 * @param[out] err an error object
 *
 * @return an error code, in the form -errno
 */
int ellis_err_code(const ellis_err *err);

/* TODO: doxygen */
const char *ellis_err_file(const ellis_err *err);

/**
 * Gets the human-readable error message associated with a given error. The
 * message object should not be directly freed but instead should be freed
 * implicitly through the actions of ellis_err_free.
 *
 * @param[out] err an error object
 *
 * @return the message
 */
const char *ellis_err_msg(const ellis_err *err);

/* TODO: doxygen */
const char *ellis_err_summary(const ellis_err *err);

/**
 * Frees an error object.
 *
 * @param[out] err an error object
 */
void ellis_err_free(const ellis_err *err);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ELLIS_ERR_H_ */
