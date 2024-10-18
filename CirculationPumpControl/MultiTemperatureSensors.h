#ifndef MULTI_TEMPERATURE_SENSORS_H
#define MULTI_TEMPERATURE_SENSORS_H

/*
  MultiTemperatureSensors.h
*/

#include <Arduino.h>
#include "DallasTemperature.h"

class MultiTemperatureSensors
{
private:
  static const uint8_t temperatur_precision = 9;

  DallasTemperature &sensors;

  DeviceAddress *const addresses;
  ssize_t *const address_params;
  uint8_t *const mappings;
  uint8_t const num_sensors;

  static const int timeout_convert = 800;
  int timeout_repeat = 9000;

  bool is_repeat_timeout;
  unsigned long time_now = 0;

  uint8_t address_count;

  bool is_started = false;

  void mapAddresses();
  inline bool isAddressMatch(uint8_t const address_index, uint8_t const params_index) const
  {
    return memcmp(((ssize_t *)(addresses + address_index)) + 1, address_params + params_index, sizeof(ssize_t)) == 0;
  }

  void (&temperatureChanged)(uint8_t, float);
  void (&addressChanged)(uint8_t, ssize_t);

public:
  MultiTemperatureSensors(
      uint8_t numSensors,
      DallasTemperature &oneWire,
      void (&temperatureChanged)(uint8_t, float),
      void (&addressChanged)(uint8_t, ssize_t));
  ~MultiTemperatureSensors();
  void start();
  void loop();
  void setRepeatTimeout(uint32_t value);
  void rescanAddresses();
  void setAddress(uint8_t index, uint32_t value);
  static constexpr float bad_temp = -127.0F;
  static const uint8_t no_mapping = 255;
  static const ssize_t min_address = 0x80000000;
  static const ssize_t max_address = 0x7fffffff;
};

#endif