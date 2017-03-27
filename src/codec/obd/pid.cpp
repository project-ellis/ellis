/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <ellis_private/codec/obd/pid.hpp>

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/system.hpp>
#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {
namespace obd {

static inline uint32_t byte_a(uint32_t x)
{
  return (x >> 24) & 0xFF;
}

static inline uint32_t byte_b(uint32_t x)
{
  return (x >> 16) & 0xFF;
}

UNUSED static inline uint32_t byte_c(uint32_t x)
{
  return (x >> 8) & 0xFF;
}

UNUSED static inline uint32_t byte_d(uint32_t x)
{
  return (x >> 0) & 0xFF;
}

/** Information used for interpreting a given PID. */
struct pid_info {
  const char *description;
  std::function<double(uint32_t)> convert;
};


double engine_coolant_temp(uint32_t val)
{
  return ((int32_t)byte_a(val)) - 40;
}


double engine_rpm(uint32_t val)
{
  return (256*byte_a(val) + byte_b(val)) / 4.0;
}


double vehicle_speed(uint32_t val)
{
  return byte_a(val);
}


double mass_air_flow(uint32_t val)
{
  return (256*byte_a(val) + byte_b(val)) / 100.0;
}


double o2_sensor_voltage(uint32_t val)
{
  return byte_a(val) / 200.0;
}


static unordered_map<uint16_t, pid_info> g_pid_map =
  {
    {0x05, {"engine_coolant_temp", engine_coolant_temp}},
    {0x0C, {"engine_rpm", engine_rpm}},
    {0x0D, {"vehicle_speed", vehicle_speed}},
    {0x10, {"mass_air_flow", mass_air_flow}},
    {0x14, {"o2_sensor_voltage", o2_sensor_voltage}},
  };


std::string get_mode_string(uint16_t mode)
{
  if (mode < 0x40) {
    /* 0x40 should be added to the mode, so something is wrong. */
    THROW_ELLIS_ERR(PARSE_FAIL, "Unknown OBD mode " << mode);
  }
  else if (mode == 0x41) {
    return string("current");
  }
  else if (mode == 0x42) {
    return string("freeze");
  }
  else {
    /* We currently support only SAE Standard modes. */
    THROW_ELLIS_ERR(PARSE_FAIL,
        "Unsupported non-SAE standard OBD mode " << mode);
  }
}


const char * get_pid_string(uint16_t pid)
{
  auto it = g_pid_map.find(pid);
  if (it == g_pid_map.end()) {
    THROW_ELLIS_ERR(PARSE_FAIL,
        "Unrecognized OBD PID " << pid);
  }

  return it->second.description;
}


double decode_value(uint16_t pid, uint32_t val)
{
  return g_pid_map.at(pid).convert(val);
}


}  /* namespace obd */
}  /* namespace ellis */
