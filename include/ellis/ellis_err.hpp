/*
 * @file ellis_err.hpp
 *
 * @brief Ellis error public C++ header.
 *
 * This is the C header for Ellis, which implements the standalone Ellis data
 * interaction routines.
 */

#pragma once
#ifndef ELLIS_ERR_HPP_
#define ELLIS_ERR_HPP_


enum class ellis_err_code {
  WRONG_TYPE = 4096,
  PARSING_ERROR = 4097,
};

class ellis_err : public std::runtime_error {

public:
  int m_code;
  std::string m_file;
  int m_line;

public:
  ellis_err(
      int code,
      const std::string &file,
      int line,
      const std::string &msg);
  ellis_err(const ellis_err &) = default;
  ~ellis_err();

  ellis_err& operator=(const ellis_err &) = default;

  /** Returns the machine-usable error code associated with the error.
   *
   * This value will be one of the normal errno values, such as EINVAL,
   * or one of the Ellis-defined values in ellis_err_code. */
  int code();

  /** Returns the filename of the location in the code that created the
   * error. */
  std::string file();

  /** Returns the line number of the location in the code that created the
   * error. */
  int line();

  /** Returns the human-readable message associated with the error.  */
  std::string msg();
  
  /** Return a summary string including code, file, line, and msg. */
  std::string summary();
};


#endif /* ELLIS_ERR_HPP_ */
