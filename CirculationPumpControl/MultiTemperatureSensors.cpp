/*
  MultiTemperatureSensor.cpp
*/

#include "MultiTemperatureSensors.h"

MultiTemperatureSensors::MultiTemperatureSensors(
    uint8_t const numSensors,
    DallasTemperature &sensors,
    void (&temperatureChanged)(uint8_t, float),
    void (&addressChanged)(uint8_t, ssize_t))
    : num_sensors(numSensors),
      sensors(sensors),
      temperatureChanged(temperatureChanged),
      addressChanged(addressChanged),
      addresses(new DeviceAddress[numSensors]),
      address_params(new ssize_t[numSensors]),
      mappings(new uint8_t[numSensors])
{
}

MultiTemperatureSensors::~MultiTemperatureSensors()
{
  delete addresses;
  delete address_params;
  delete mappings;
}

void MultiTemperatureSensors::start(void)
{
  sensors.begin();

  rescanAddresses();

  time_now = millis();
  is_repeat_timeout = true;
  is_started = true;
}

void MultiTemperatureSensors::loop(void)
{
  if (is_repeat_timeout)
  {
    if (millis() - time_now > timeout_repeat)
    {
      time_now = millis();
      is_repeat_timeout = false;

      sensors.requestTemperatures();
    }
  }
  else if (millis() - time_now > timeout_convert)
  {
    time_now = millis();
    is_repeat_timeout = true;

    for (uint8_t i = 0; i < num_sensors; i++)
    {
      uint8_t mapping = mappings[i];
      temperatureChanged(
          i,
          mapping < address_count
              ? sensors.getTempC(addresses[mapping])
              : bad_temp);
    }
  }
}

void MultiTemperatureSensors::setAddress(uint8_t const index, uint32_t const value)
{
  if (index < num_sensors)
  {
    address_params[index] = value;
    if (is_started)
    {
      mapAddresses();
    }
  }
}

void MultiTemperatureSensors::setRepeatTimeout(uint32_t const value)
{
  timeout_repeat = value - timeout_convert;
}

void MultiTemperatureSensors::mapAddresses()
{
  address_count = 0;
  memset(mappings, no_mapping, num_sensors);

  uint8_t bus_count = sensors.getDeviceCount();

  bool hasAddressMatch;
  for (uint8_t bus_index = 0; bus_index < bus_count && address_count < num_sensors; bus_index++)
  {
    if (sensors.getAddress(addresses[address_count], bus_index))
    {
      hasAddressMatch = false;
      for (uint8_t params_index = 0; params_index < num_sensors; params_index++)
      {
        if (isAddressMatch(address_count, params_index))
        {
          mappings[params_index] = address_count;
          hasAddressMatch = true;
        }
      }
      if (hasAddressMatch)
      {
        address_count++;
      }
    }
    else
    {
      break;
    }
  }

  for (uint8_t i = 0; i < address_count; i++)
  {
    sensors.setResolution(addresses[i], temperatur_precision);
  }
}

void MultiTemperatureSensors::rescanAddresses()
{
  mapAddresses();

  uint8_t bus_count = sensors.getDeviceCount();

  if (address_count == bus_count || address_count == num_sensors)
  {
    return;
  }

  bool hasAddressMatch;
  uint8_t unmapped_index;

  for (uint8_t bus_index = 0; bus_index < bus_count && address_count < num_sensors; bus_index++)
  {
    if (sensors.getAddress(addresses[address_count], bus_index))
    {
      hasAddressMatch = false;
      unmapped_index = no_mapping;
      for (uint8_t params_index = 0; !hasAddressMatch && params_index < num_sensors; params_index++)
      {
        if (mappings[params_index] < num_sensors)
        {
          hasAddressMatch |= isAddressMatch(address_count, params_index);
        }
        else
        {
          unmapped_index = params_index;
        }
      }
      if (!hasAddressMatch && unmapped_index < num_sensors)
      {
        sensors.setResolution(addresses[address_count], temperatur_precision);

        mappings[unmapped_index] = address_count;
        addressChanged(
            address_count,
            *(((ssize_t *)(addresses + address_count)) + 1));

        address_count++;
      }
    }
    else
    {
      break;
    }
  }
}
