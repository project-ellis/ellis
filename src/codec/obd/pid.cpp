#include <ellis/private/codec/obd/pid.hpp>

#include <ellis/core/defs.hpp>
#include <ellis/private/using.hpp>

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


std::unique_ptr<std::string> get_mode_string(uint16_t mode)
{
  unique_ptr<string> mode_str = nullptr;
  if (mode == 0x7F) {
    mode_str.reset(new string("unknown"));
  }
  else if (mode < 0x40) {
    /* 0x40 should be added to the mode, so something is wrong. */
    return nullptr;
  }
  else if (mode == 0x41) {
    mode_str.reset(new string("current"));
  }
  else if (mode == 0x42) {
    mode_str.reset(new string("freeze"));
  }
  else {
    /* TODO: throw unsupported if it's a mode we don't yet handle*/
    mode_str.reset(new string(std::to_string(mode - 0x40)));
  }

  return mode_str;
}


const char * get_pid_string(uint16_t pid)
{
  auto it = g_pid_map.find(pid);
  if (it == g_pid_map.end()) {
    return nullptr;
  }

  return it->second.description;
}


double decode_value(uint16_t pid, uint32_t val)
{
  return g_pid_map.at(pid).convert(val);
}


}  /* namespace obd */
}  /* namespace ellis */
