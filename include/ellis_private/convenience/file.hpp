/*
 * @file ellis_private/convenience/file.hpp
 *
 * @brief convenience routines for working with files
 *
 */

#pragma once
#ifndef ELLIS_PRIVATE_CONVENIENCE_FILE_HPP_
#define ELLIS_PRIVATE_CONVENIENCE_FILE_HPP_


#include <stdio.h>
#include <string>


/**
 * Return the extension portion of a filename.
 *
 * If no extension is present, throws ellis::err (INVALID_ARGS).
 */
std::string get_extension(const char *filename);


#endif /* ELLIS_PRIVATE_CONVENIENCE_FILE_HPP_ */
