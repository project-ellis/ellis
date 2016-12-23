/*
 * @file ellis/core/disposition.hpp
 *
 * @brief Ellis disposition class C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_DISPOSITION_HPP_
#define ELLIS_CORE_DISPOSITION_HPP_

#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/system.hpp>
#include <memory>

namespace ellis {


/*  ____  _                     _ _   _
 * |  _ \(_)___ _ __   ___  ___(_) |_(_) ___  _ __
 * | | | | / __| '_ \ / _ \/ __| | __| |/ _ \| '_ \
 * | |_| | \__ \ |_) | (_) \__ \ | |_| | (_) | | | |
 * |____/|_|___/ .__/ \___/|___/_|\__|_|\___/|_| |_|
 *             |_|
 */


/**
 * This templated class is used for returning:
 *
 * - the state of processing (or failure thereof) (always present)
 * - (in case of error) error details
 * - (in case of success) the finished value
 *
 * It is templated on state, value, and error type, for flexibility.
 *
 * STATE must be an enum class that has at least a SUCCESS and ERROR entry.
 *
 * VALUE and ERRTYPE must be move-copyable.  One way to guarantee this is
 * to use std::unique_ptr.
 */
template <typename STATE, typename VALUE, typename ERRTYPE>
class disposition {
  STATE m_state;
  union {
    VALUE m_res;
    ERRTYPE m_err;
  };

public:
  explicit disposition(ERRTYPE e) :
    m_state(STATE::ERROR)
  {
    new (&m_err) ERRTYPE(std::move(e));
  }

  explicit disposition(VALUE r) :
    m_state(STATE::SUCCESS)
  {
    new (&m_res) VALUE(std::move(r));
  }

  explicit disposition(STATE st) :
    m_state(st)
  {
    ELLIS_ASSERT(st != STATE::ERROR);
    ELLIS_ASSERT(st != STATE::SUCCESS);
  }

  disposition(disposition &&o) noexcept
  {
    *this = std::move(o);
  }

  ~disposition() noexcept
  {
    if (m_state == STATE::ERROR) {
      m_err.~ERRTYPE();
    }
    else if (m_state == STATE::SUCCESS) {
      m_res.~VALUE();
    }
  }

  disposition & operator=(disposition &&o) noexcept
  {
    if (this != &o) {
      m_state = o.m_state;
      o.m_state = STATE::CONTINUE;
      if (m_state == STATE::ERROR) {
        new (&m_err) ERRTYPE(std::move(o.m_err));
      }
      else if (m_state == STATE::SUCCESS) {
        new (&m_res) VALUE(std::move(o.m_res));
      }
    }
    return *this;
  }

  STATE state() const
  {
    return m_state;
  }

  ERRTYPE extract_error() {
    if (m_state == STATE::ERROR) {
      return std::move(m_err);
    } else {
      return {};
    }
  }

  VALUE extract_value() {
    if (m_state == STATE::SUCCESS) {
      return std::move(m_res);
    } else {
      return {};
    }
  }
};  /* End of templatized disposition class. */


/* Common types of dispositions. */

using node_progress =
  disposition<stream_state, std::unique_ptr<node>, std::unique_ptr<err>>;


}  /* namespace ellis */

#endif  /* ELLIS_CORE_DISPOSITION_HPP_ */
