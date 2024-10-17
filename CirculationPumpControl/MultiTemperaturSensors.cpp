/*
  MultiTemperatureSensor.ino
*/

#include "MultiTemperatureSensors.h"

MultiTemperatureSensors::MultiTemperatureSensors(OneWire* oneWire, uint8_t baseChannel, uint8_t repeatParam, uint8_t rescanParam, uint8_t addressParams)
{
    sensors = DallasTemperature(oneWire);
    base_channel = baseChannel;
    repeat_param = repeatParam;
    rescan_param = rescanParam;
    address_param_base = addressParams;
}

void MultiTemperatureSensors::setup(void)
{
	sensors.begin();

	for (uint8_t i = 0; i < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; i++)
	{
		address_params[i] = zunoLoadCFGParam(address_param_base + i);
	}

	rescanAddresses();

	setRepeatTimeout(zunoLoadCFGParam(repeat_param));

	time_now = millis();
	is_repeat_timeout = true;
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

		for (uint8_t i = 0; i < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; i++)
		{
			uint8_t mapping = mappings[i];
			temperatures[i] = mapping < address_count
								  ? sensors.getTempC(addresses[mapping]) * 10.0
								  : bad_temp;
			zunoSendReport(base_channel + i);
		}
	}
}

void MultiTemperatureSensors::setParameter(uint8_t param, uint32_t value)
{
	if (param == repeat_param)
	{
		setRepeatTimeout(value);
	}
	else if (param == rescan_param)
	{
		rescanAddresses();
	}
	else if (param >= address_param_base)
	{
		uint8_t index = param - address_param_base;
		if (index < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS)
		{
			address_params[index] = value;
		}
		mapAddresses();
	}
}

void MultiTemperatureSensors::setRepeatTimeout(uint32_t value)
{
	timeout_repeat = value * 1000 - timeout_convert;
}

void MultiTemperatureSensors::mapAddresses()
{
	address_count = 0;
	memset(mappings, no_mapping, MULTI_TEMPERATURE_SENSORS_MAX_SENSORS);

	uint8_t bus_count = sensors.getDeviceCount();

	bool hasAddressMatch;
	for (uint8_t bus_index = 0; bus_index < bus_count && address_count < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; bus_index++)
	{
		if (sensors.getAddress(addresses[address_count], bus_index))
		{
			hasAddressMatch = false;
			for (uint8_t params_index = 0; params_index < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; params_index++)
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

	if (address_count == bus_count || address_count == MULTI_TEMPERATURE_SENSORS_MAX_SENSORS)
	{
		return;
	}

	bool hasAddressMatch;
	uint8_t unmapped_index;

	for (uint8_t bus_index = 0; bus_index < bus_count && address_count < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; bus_index++)
	{
		if (sensors.getAddress(addresses[address_count], bus_index))
		{
			hasAddressMatch = false;
			unmapped_index = no_mapping;
			for (uint8_t params_index = 0; !hasAddressMatch && params_index < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; params_index++)
			{
				if (mappings[params_index] < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS)
				{
					hasAddressMatch |= isAddressMatch(address_count, params_index);
				}
				else
				{
					unmapped_index = params_index;
				}
			}
			if (!hasAddressMatch && unmapped_index < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS)
			{
				sensors.setResolution(addresses[address_count], temperatur_precision);

				mappings[unmapped_index] = address_count;
				zunoSaveCFGParam(address_param_base + address_count, *(((ssize_t *)(addresses + address_count)) + 1));

				address_count++;
			}
		}
		else
		{
			break;
		}
	}
}
