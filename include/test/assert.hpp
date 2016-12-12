/*
 * @file test/assert.hpp
 *
 * @brief Test assertion code.
 */

#pragma once
#ifndef ELLIS_TEST_ASSERT_HPP_
#define ELLIS_TEST_ASSERT_HPP_



#include <cfloat>

namespace ellis {
namespace test {

bool dbl_equal(double a, double b)
{
  return std::abs(a - b) <= DBL_EPSILON;
}


}  /* namespace test */
}  /* namespace ellis */

#endif  /* ELLIS_TEST_ASSERT_HPP_ */

