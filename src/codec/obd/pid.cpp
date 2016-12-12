#include <ellis/private/codec/obd/pid.hpp>

#include <ellis/private/using.hpp>

#define BYTE_A(X) ((X >> 24) & 0xFF)
#define BYTE_B(X) ((X >> 16) & 0xFF)
#define BYTE_C(X) ((X >> 8) & 0xFF)
#define BYTE_D(X) ((X >> 0) & 0xFF)

namespace ellis {
namespace obd {

struct pid_info {
  const char *description;
  std::function<double(uint32_t)> convert;
};


double engine_coolant_temp(uint32_t val)
{
  return ((int32_t)BYTE_A(val)) - 40;
}


double engine_rpm(uint32_t val)
{
  return (256*BYTE_A(val) + BYTE_B(val)) / 4.0;
}


double vehicle_speed(uint32_t val)
{
  return BYTE_A(val);
}


double mass_air_flow(uint32_t val)
{
  return (256*BYTE_A(val) + BYTE_B(val)) / 100.0;
}


double o2_sensor_voltage(uint32_t val)
{
  return BYTE_A(val) / 200.0;
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
