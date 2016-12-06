/*
 * @file private/err.hpp
 *
 * @brief Ellis error private C++ header.
 *
 * This is the C++ header for Ellis errors.
 */

#pragma once
#ifndef ELLIS_PRIVATE_ERR_HPP_
#define ELLIS_PRIVATE_ERR_HPP_

namespace ellis {


#define MAKE_ELLIS_ERR(CODE, MSG) \
  err(CODE, __FILE__, __LINE__, (MSG))


}  /* namespace ellis */

#endif  /* ELLIS_PRIVATE_ERR_HPP_ */
