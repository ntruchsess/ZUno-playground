/*
  MultiTemperatureSensor.cpp
*/

#include "MultiTemperatureSensors.h"

MultiTemperatureSensors::MultiTemperatureSensors(
	DallasTemperature *sensors,
	void (*temperatureChanged)(uint8_t, float),
	void (*addressChanged)(uint8_t, ssize_t))
	: sensors(sensors),
	  temperatureChanged(temperatureChanged),
	  addressChanged(addressChanged)
{
}

void MultiTemperatureSensors::start(void)
{
	sensors->begin();

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

			sensors->requestTemperatures();
		}
	}
	else if (millis() - time_now > timeout_convert)
	{
		time_now = millis();
		is_repeat_timeout = true;

		for (uint8_t i = 0; i < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; i++)
		{
			uint8_t mapping = mappings[i];
			(*temperatureChanged)(
				i,
				mapping < address_count
					? sensors->getTempC(addresses[mapping]) * 10.0
					: bad_temp);
		}
	}
}

void MultiTemperatureSensors::setAddress(uint8_t index, uint32_t value)
{
	if (index < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS)
	{
		address_params[index] = value;
		if (is_started)
		{
			mapAddresses();
		}
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

	uint8_t bus_count = sensors->getDeviceCount();

	bool hasAddressMatch;
	for (uint8_t bus_index = 0; bus_index < bus_count && address_count < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; bus_index++)
	{
		if (sensors->getAddress(addresses[address_count], bus_index))
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
		sensors->setResolution(addresses[i], temperatur_precision);
	}
}

void MultiTemperatureSensors::rescanAddresses()
{
	mapAddresses();

	uint8_t bus_count = sensors->getDeviceCount();

	if (address_count == bus_count || address_count == MULTI_TEMPERATURE_SENSORS_MAX_SENSORS)
	{
		return;
	}

	bool hasAddressMatch;
	uint8_t unmapped_index;

	for (uint8_t bus_index = 0; bus_index < bus_count && address_count < MULTI_TEMPERATURE_SENSORS_MAX_SENSORS; bus_index++)
	{
		if (sensors->getAddress(addresses[address_count], bus_index))
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
				sensors->setResolution(addresses[address_count], temperatur_precision);

				mappings[unmapped_index] = address_count;
				(*addressChanged)(
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
