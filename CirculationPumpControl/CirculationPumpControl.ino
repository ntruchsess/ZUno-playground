/*
  CirculationPumpControl.ino
*/

#include "ZUNO_OneWire.h"
#include "MultiTemperatureSensors.h"

enum
{
  REPEAT_PARAM = 64,
  RESCAN_ADDRESSES_PARAM,
  ADDRESS_PARAM
};

#define MAX_SENSORS 3

int16_t temperatures[MAX_SENSORS];

ZUNO_SETUP_CHANNELS(
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[0]),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[1]),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[2]));

ZUNO_SETUP_CONFIGPARAMETERS(
    ZUNO_CONFIG_PARAMETER_2B_INFO_SIGN("SensorInterval", "temperature mesurement repeat interval in seconds", 1, 3600, 10),
    ZUNO_CONFIG_PARAMETER_1B_INFO("Rescan", "rescan the 1-Wire bus", 0, 1, 0),
    ZUNO_CONFIG_PARAMETER_INFO_SIGN("SensorHeater", "address of sensor at heater exit", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0),
    ZUNO_CONFIG_PARAMETER_INFO_SIGN("SensorMixer", "address of sensor at mixer exit", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0),
    ZUNO_CONFIG_PARAMETER_INFO_SIGN("SensorReturn", "address of sensor at return from circulation", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0));

ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);

#define ONE_WIRE_BUS 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

MultiTemperatureSensors multiSensors(MAX_SENSORS, sensors, temperatureChanged, addressChanged);

void setup(void)
{
  for (uint8_t index = 0; index < MAX_SENSORS; index++)
  {
    multiSensors.setAddress(index, zunoLoadCFGParam(index + ADDRESS_PARAM));
  }
  multiSensors.setRepeatTimeout(zunoLoadCFGParam(REPEAT_PARAM));
  multiSensors.start();
}

void loop(void)
{
  multiSensors.loop();
}

void config_parameter_changed(uint8_t const param, uint32_t const value)
{
  if (param == REPEAT_PARAM)
  {
    multiSensors.setRepeatTimeout(value);
  }
  else if (param == RESCAN_ADDRESSES_PARAM)
  {
    multiSensors.rescanAddresses();
  }
  else if (param >= ADDRESS_PARAM)
  {
    multiSensors.setAddress(param - ADDRESS_PARAM, value);
  }
}

void temperatureChanged(uint8_t const index, float const temperature)
{
  if (index < MAX_SENSORS)
  {
    temperatures[index] = temperature * 10.0;
    zunoSendReport(index);
  }
}

void addressChanged(uint8_t const index, ssize_t const address)
{
  if (index < MAX_SENSORS)
  {
    zunoSaveCFGParam(index + ADDRESS_PARAM, address);
  }
}
