/*
 * @file ellis/core/err.hpp
 *
 * @brief Ellis error public C++ header.
 *
 * This is the C++ header for Ellis errors.
 */

#pragma once
#ifndef ELLIS_CORE_ERR_HPP_
#define ELLIS_CORE_ERR_HPP_

#include <stdexcept>
#include <string>


namespace ellis {


enum class err_code {
  WRONG_TYPE = 4096,
  TYPE_MISMATCH = 4097,
  PARSING_ERROR = 4098,
  NOT_MERGED = 4099,
  PATH_ERROR = 4100,
  TODO = 4101
};


class err : public std::runtime_error {

public:
  err_code m_code;
  std::string m_file;
  int m_line;

public:
  err(
      err_code code,
      const std::string &file,
      int line,
      const std::string &msg);
  err(const err &) = default;
  ~err();

  err& operator=(const err &) = default;

  /** Returns the machine-usable error code associated with the error.
   */
  err_code code() const;

  /** Returns the filename of the location in the code that created the
   * error. */
  std::string file() const;

  /** Returns the line number of the location in the code that created the
   * error. */
  int line() const;

  /** Returns the human-readable message associated with the error.  */
  std::string msg() const;

  /** Return a summary string including code, file, line, and msg. */
  std::string summary() const;
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_ERR_HPP_ */
