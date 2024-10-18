/*
  CirculationPumpControl.ino
*/

#include "ZUNO_OneWire.h"
#include "MultiTemperatureSensors.h"
#include "CircControl.h"

enum
{
  CHANNEL_OUTPUT = 1,
  CHANNEL_TRIGGER,
  CHANNEL_TEMPERATURE
};

enum
{
  MIN_RUNTIME_PARAM = 64,
  MAX_RUNTIME_PARAM,
  MAX_TEMPERATURE_DIFFERENCE_PARAM,
  REPEAT_PARAM,
  FILTER_THRESHOLD_PARAM,
  RESCAN_ADDRESSES_PARAM,
  ADDRESS_PARAM
};

#define MAX_SENSORS 3

int16_t temperatures[MAX_SENSORS];
bool triggerState = false;

ZUNO_SETUP_CHANNELS(
    ZUNO_SWITCH_BINARY(getOutputState, setOutputState),
    ZUNO_SWITCH_BINARY(getTrigger, setTrigger),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[0]),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[1]),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[2]));

ZUNO_SETUP_CONFIGPARAMETERS(
    ZUNO_CONFIG_PARAMETER_2B_INFO("Min Pump Runtime", "minimum runtime (seconds)", 1, 3600, 10),
    ZUNO_CONFIG_PARAMETER_2B_INFO("Max Pump Runtime", "maximum runtime (seconds)", 1, 3600, 300),
    ZUNO_CONFIG_PARAMETER_2B_INFO_SIGN("Max Temp Difference", "maximum temperature difference (10*°C)", 5, 500, 100),
    ZUNO_CONFIG_PARAMETER_INFO("SensorInterval", "temperature mesurement repeat interval (milliseconds)", 1000, 3600000, 2000),
    ZUNO_CONFIG_PARAMETER_2B_INFO_SIGN("Filter Threshold", "threshold for heater sensor filter", 0x8000, 0x7fff, 315),
    ZUNO_CONFIG_PARAMETER_1B_INFO("Rescan", "rescan the 1-Wire bus", 0, 1, 0),
    ZUNO_CONFIG_PARAMETER_INFO_SIGN("HeaterAddress", "Address of sensor at heater exit", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0),
    ZUNO_CONFIG_PARAMETER_INFO_SIGN("MixerAddress", "Address of sensor at mixer exit", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0),
    ZUNO_CONFIG_PARAMETER_INFO_SIGN("ReturnAddress", "Address of sensor at return from circulation", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0));

ZUNO_SETUP_CFGPARAMETER_HANDLER(zunoConfigChanged);

#define ONE_WIRE_BUS 9
#define OUTPUT_PIN 13

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#ifdef CIRC_CONTROL_DEBUG
CircControl circControl(onCircControlPumpStatus, Serial);
#else
CircControl circControl(onCircControlPumpStatus);
#endif

MultiTemperatureSensors multiSensors(MAX_SENSORS, sensors, temperatureChanged, addressChanged);

void setup(void)
{
#ifdef CIRC_CONTROL_DEBUG
  Serial.begin(115200);
#endif

  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);

  circControl.setMinRunTime(zunoLoadCFGParam(MIN_RUNTIME_PARAM));
  circControl.setMaxRunTime(zunoLoadCFGParam(MAX_RUNTIME_PARAM));
  circControl.setMaxTempDifference(zunoLoadCFGParam(MAX_TEMPERATURE_DIFFERENCE_PARAM));
  circControl.setFilterThreshold(zunoLoadCFGParam(FILTER_THRESHOLD_PARAM));

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
  circControl.loop();
}

void zunoConfigChanged(uint8_t const param, uint32_t const value)
{
  switch (param)
  {
  case MIN_RUNTIME_PARAM:
    circControl.setMinRunTime(value);
    break;
  case MAX_RUNTIME_PARAM:
    circControl.setMaxRunTime(value);
    break;
  case MAX_TEMPERATURE_DIFFERENCE_PARAM:
    circControl.setMaxTempDifference(value);
    break;
  case REPEAT_PARAM:
    multiSensors.setRepeatTimeout(value);
    break;
  case FILTER_THRESHOLD_PARAM:
    circControl.setFilterThreshold(value);
    break;
  case RESCAN_ADDRESSES_PARAM:
    multiSensors.rescanAddresses();
    break;
  default:
    if (param >= ADDRESS_PARAM)
    {
      multiSensors.setAddress(param - ADDRESS_PARAM, value);
    }
    break;
  }
}

byte getOutputState()
{
  return circControl.getPumpStatus() ? 255 : 0;
}

void setOutputState(byte const state)
{
  if (state == 0)
  {
    circControl.stopPump();
  }
  else
  {
    circControl.startPump();
  }
}

byte getTrigger()
{
  return triggerState ? 255 : 0;
}

void setTrigger(byte trigger)
{
  triggerState = trigger != 0;
  circControl.startPump();
}

void temperatureChanged(uint8_t const index, float const temperature)
{
  if (index < MAX_SENSORS)
  {
    int16_t temp = temperature * 10.0;
    if (index == 0)
    {
      circControl.setHeaterTemperature(temp);
    }
    else if (index == 1)
    {
      circControl.setMixerTemperature(temp);
    }
    else if (index == 2)
    {
      circControl.setReturnTemperature(temp);
    }
    temperatures[index] = temp;
    zunoSendReport(index + CHANNEL_TEMPERATURE);
  }
}

void addressChanged(uint8_t const index, ssize_t const address)
{
  if (index < MAX_SENSORS)
  {
    zunoSaveCFGParam(index + ADDRESS_PARAM, address);
  }
}

void onCircControlPumpStatus(const bool state)
{
  digitalWrite(OUTPUT_PIN, state ? HIGH : LOW);
  zunoSendReport(CHANNEL_OUTPUT);
}
