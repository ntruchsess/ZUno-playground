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

ZUNO_SETUP_CHANNELS(
	ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, getTemperature1),
	ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, getTemperature2),
	ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, getTemperature3));

ZUNO_SETUP_CONFIGPARAMETERS(
	ZUNO_CONFIG_PARAMETER_2B_INFO_SIGN("SensorInterval", "temperature mesurement repeat interval in seconds", 1, 3600, 10),
	ZUNO_CONFIG_PARAMETER_1B_INFO("Rescan", "rescan the 1-Wire bus", 0, 1, 0),
	ZUNO_CONFIG_PARAMETER_INFO_SIGN("SensorAddress1", "address of sensor at heater exit", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0),
	ZUNO_CONFIG_PARAMETER_INFO_SIGN("SensorAddress2", "address of sensor at mixer exit", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0),
	ZUNO_CONFIG_PARAMETER_INFO_SIGN("SensorAddress3", "address of sensor at return from circulation", MultiTemperatureSensors::min_address, MultiTemperatureSensors::max_address, 0));

ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);

#define ONE_WIRE_BUS 9

OneWire oneWire(ONE_WIRE_BUS);

MultiTemperatureSensors multiSensors(&oneWire, 1, REPEAT_PARAM, RESCAN_ADDRESSES_PARAM, ADDRESS_PARAM);

void setup(void)
{
	multiSensors.setup();
}

void loop(void)
{
	multiSensors.loop();
}

void config_parameter_changed(uint8_t param, uint32_t value)
{
	multiSensors.setParameter(param, value);
}

int16_t getTemperature1()
{
	return multiSensors.getTemperature(0);
}

int16_t getTemperature2()
{
	return multiSensors.getTemperature(1);
}

int16_t getTemperature3()
{
	return multiSensors.getTemperature(2);
}