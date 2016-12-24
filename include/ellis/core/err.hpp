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

#include <ellis/core/system.hpp>
#include <stdexcept>
#include <string>


namespace ellis {


/*   ____                           _
 *  / ___|___  _ ____   _____ _ __ (_) ___ _ __   ___ ___
 * | |   / _ \| '_ \ \ / / _ \ '_ \| |/ _ \ '_ \ / __/ _ \
 * | |__| (_) | | | \ V /  __/ | | | |  __/ | | | (_|  __/
 *  \____\___/|_| |_|\_/ \___|_| |_|_|\___|_| |_|\___\___|
 *
 *
 *  _ __ ___   __ _  ___ _ __ ___  ___
 * | '_ ` _ \ / _` |/ __| '__/ _ \/ __|
 * | | | | | | (_| | (__| | | (_) \__ \
 * |_| |_| |_|\__,_|\___|_|  \___/|___/
 *
 *
 * (Using these macros will improve maintainability).
 */


/**
 * Make an instance of ellis::err, using the provided error code and text.
 *
 * File, line, and function are automatically supplied based on the
 * location in the code that called MAKE_ELLIS_ERR.
 *
 * The text may be a simple string, or a series << of << items to be sent to
 * a stringstream in order to create the string, ala ELLIS_LOGS.
 *
 * For example:
 *
 *   e = MAKE_ELLIS_ERR(INVALID_ARGS, "x=" << x << " clashes w/ t=" << t);
 *
 * For throwing an ellis:err, please use the THROW_ELLIS_ERR macro below.
 */
#define MAKE_ELLIS_ERR(CODE, MSG) \
  ellis::err(ellis::err_code::CODE, \
      __FILE__, __LINE__, __FUNCTION__, \
      ELLIS_SSTRING("ERROR:" #CODE ": " << MSG))

/**
 * Make a unique_ptr to an ellis::err object.
 *
 * See MAKE_ELLIS_ERR, which is similar, for additional info.
 */
#define MAKE_UNIQUE_ELLIS_ERR(CODE, MSG) \
  std::make_unique<ellis::err>(ellis::err_code::CODE, \
      __FILE__, __LINE__, __FUNCTION__, \
      ELLIS_SSTRING("ERROR:" #CODE ": " << MSG))

/**
 * Throw an ellis:err.
 *
 * See MAKE_ELLIS_ERR, which is similar, for additional info.
 */
#define THROW_ELLIS_ERR(CODE, MSG) \
  throw MAKE_ELLIS_ERR(CODE, MSG)



/*  _____                                     _
 * | ____|_ __ _ __ ___  _ __    ___ ___   __| | ___  ___
 * |  _| | '__| '__/ _ \| '__|  / __/ _ \ / _` |/ _ \/ __|
 * | |___| |  | | | (_) | |    | (_| (_) | (_| |  __/\__ \
 * |_____|_|  |_|  \___/|_|     \___\___/ \__,_|\___||___/
 *
 */


enum class err_code {
  /* Abnormal errors, which usually require bug fixes. */
  TODO = 4101,          //<<< For rapid prototyping only, must replace...
  INTERNAL = 4102,      //<<< A non-asserting internal problem (e.g. in codec).

  /* Probably the fault of the callee. */
  COLLISION = 4201,     //<<< Unexpected collision that should not occur.
  OVER_LOAD = 4202,     //<<< System too busy, but request might succeed later.
  OVER_CAPACITY = 4203, //<<< Capacity has been exhausted, e.g. storage space.
  IO = 4204,            //<<< I/O error.

  /* Probably the fault of the caller. */
  TYPE_MISMATCH = 4301, //<<< Parameter(s) have bad or mismatched type.
  INVALID_ARGS = 4302,  //<<< Parameter(s) are invalid (singly or collectively).
  POLICY_FAIL = 4303,   //<<< Violation of policy (e.g. map merge policy).
  NOT_PERMITTED = 4304, //<<< Insufficient privileges.
  NO_SUCH = 4305,       //<<< The specified entity is non-existant.
  ALREADY = 4306,       //<<< The specified entity already present.
  EXPIRED = 4307,       //<<< Expiration exceeded, or timeout expired.
  SCHEMA_FAIL = 4308,   //<<< Violation of schema.
  PARSE_FAIL = 4309,    //<<< Invalid bytestream presented to decoder.
  TRANSLATE_FAIL = 4310,//<<< Encoding format can not express the value(s).
  PATH_FAIL = 4311,     //<<< Invalid path.
};



/*  _____                             _     _           _
 * | ____|_ __ _ __ ___  _ __    ___ | |__ (_) ___  ___| |_
 * |  _| | '__| '__/ _ \| '__|  / _ \| '_ \| |/ _ \/ __| __|
 * | |___| |  | | | (_) | |    | (_) | |_) | |  __/ (__| |_
 * |_____|_|  |_|  \___/|_|     \___/|_.__// |\___|\___|\__|
 *                                       |__/
 */


class err : public std::runtime_error {

public:
  err_code m_code;
  std::string m_file;
  int m_line;
  std::string m_func;

public:
  err(
      err_code code,
      const std::string &file,
      int line,
      const std::string &func,
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

  /** Returns the function name of the location in the code that created the
   * error. */
  std::string func() const;

  /** Returns the human-readable message associated with the error.  */
  std::string msg() const;

  /** Return a summary string including code, file, line, and msg. */
  std::string summary() const;
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_ERR_HPP_ */
