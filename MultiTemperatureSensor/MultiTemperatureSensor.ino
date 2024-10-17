/*
  MultiTemperatureSensor.ino
*/

#include "ZUNO_OneWire.h"
#include "DallasTemperature.h"

enum
{
	REPEAT_PARAM = 64,
	RESCAN_ADDRESSES_PARAM,
	ADDRESS_PARAM
};

const uint8_t max_sensors = 3;
const ssize_t min_address = 0x80000000;
const ssize_t max_address = 0x7fffffff;
const int16_t bad_temp = -1270;
const uint8_t no_mapping = 255;

int16_t temperatures[max_sensors];

ZUNO_SETUP_CHANNELS(
	ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[0]),
	ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[1]),
	ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, temperatures[2]));

ZUNO_SETUP_CONFIGPARAMETERS(
	ZUNO_CONFIG_PARAMETER_2B_SIGN("RepeatInterval", 1, 3600, 10),
	ZUNO_CONFIG_PARAMETER_1B("Rescan", 0, 1, 0),
	ZUNO_CONFIG_PARAMETER_SIGN("Address1", min_address, max_address, 0),
	ZUNO_CONFIG_PARAMETER_SIGN("Address2", min_address, max_address, 0),
	ZUNO_CONFIG_PARAMETER_SIGN("Address3", min_address, max_address, 0));

ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);

#define ONE_WIRE_BUS 9
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress addresses[max_sensors];
ssize_t address_params[max_sensors];
uint8_t mappings[max_sensors];

int timeout_convert = 1000;
int timeout_repeat = 9000;

bool is_repeat_timeout;
unsigned long time_now = 0;

uint8_t address_count;

void setup(void)
{
	sensors.begin();

	for (uint8_t i = 0; i < max_sensors; i++)
	{
		address_params[i] = zunoLoadCFGParam(ADDRESS_PARAM + i);
	}

	rescanAddresses();

	setRepeatTimeout(zunoLoadCFGParam(REPEAT_PARAM));

	time_now = millis();
	is_repeat_timeout = true;
}

void loop(void)
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

		for (uint8_t i = 0; i < max_sensors; i++)
		{
			uint8_t mapping = mappings[i];
			temperatures[i] = mapping < address_count
								  ? sensors.getTempC(addresses[mapping]) * 10.0
								  : bad_temp;
			zunoSendReport(i + 1);
		}
	}
}

void config_parameter_changed(uint8_t param, uint32_t value)
{
	if (param == REPEAT_PARAM)
	{
		setRepeatTimeout(value);
	}
	else if (param == RESCAN_ADDRESSES_PARAM)
	{
		rescanAddresses();
	}
	else if (param >= ADDRESS_PARAM)
	{
		uint8_t index = param - ADDRESS_PARAM;
		if (index < max_sensors)
		{
			address_params[index] = value;
		}
		mapAddresses();
	}
}

void setRepeatTimeout(uint32_t value)
{
	timeout_repeat = value * 1000 - timeout_convert;
}

void mapAddresses()
{
	address_count = 0;
	memset(mappings, no_mapping, max_sensors);

	uint8_t bus_count = sensors.getDeviceCount();

	bool hasAddressMatch;
	for (uint8_t bus_index = 0; bus_index < bus_count && address_count < max_sensors; bus_index++)
	{
		if (sensors.getAddress(addresses[address_count], bus_index))
		{
			hasAddressMatch = false;
			for (uint8_t params_index = 0; params_index < max_sensors; params_index++)
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
		sensors.setResolution(addresses[i], TEMPERATURE_PRECISION);
	}
}

void rescanAddresses()
{
	mapAddresses();

	uint8_t bus_count = sensors.getDeviceCount();

	if (address_count == bus_count || address_count == max_sensors)
	{
		return;
	}

	bool hasAddressMatch;
	uint8_t unmapped_index;

	for (uint8_t bus_index = 0; bus_index < bus_count && address_count < max_sensors; bus_index++)
	{
		if (sensors.getAddress(addresses[address_count], bus_index))
		{
			hasAddressMatch = false;
			unmapped_index = no_mapping;
			for (uint8_t params_index = 0; !hasAddressMatch && params_index < max_sensors; params_index++)
			{
				if (mappings[params_index] < max_sensors)
				{
					hasAddressMatch |= isAddressMatch(address_count, params_index);
				}
				else
				{
					unmapped_index = params_index;
				}
			}
			if (!hasAddressMatch && unmapped_index < max_sensors)
			{
				sensors.setResolution(addresses[address_count], TEMPERATURE_PRECISION);

				mappings[unmapped_index] = address_count;
				zunoSaveCFGParam(ADDRESS_PARAM + address_count, *(((ssize_t *)(addresses + address_count)) + 1));

				address_count++;
			}
		}
		else
		{
			break;
		}
	}
}

inline bool isAddressMatch(uint8_t address_index, uint8_t params_index)
{
	return memcmp(((ssize_t *)(addresses + address_index)) + 1, address_params + params_index, sizeof(ssize_t)) == 0;
}
