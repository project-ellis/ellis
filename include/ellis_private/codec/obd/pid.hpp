/*
 * @file codec/elm327.hpp
 *
 * @brief Ellis ELM327 codec C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_PID_HPP_
#define ELLIS_CODEC_PID_HPP_

#include <cstdint>
#include <memory>

namespace ellis {
namespace obd {


/** Gets a string describing a PID from a PID value.
 *
 * @param pid a PID
 *
 * @return a string description
 * */
const char * get_pid_string(uint16_t pid);

/** Gets a string describing a PID mode from a mode value.
 *
 * @param mode a PID mode
 *
 * @return a string description
 */
std::unique_ptr<std::string> get_mode_string(uint16_t mode);

/** Decodes a given value, applying the appropriate unit transformation
 * depending on the given PID.
 *
 * @param pid a PID
 * @param value an OBD II value
 *
 * @return a decoded value
 */
double decode_value(uint16_t pid, uint32_t val);


}  /* obd */
}  /* ellis */

#endif  /* ELLIS_CODEC_PID_HPP_ */
