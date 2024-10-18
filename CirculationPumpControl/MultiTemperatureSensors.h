#ifndef MULTI_TEMPERATURE_SENSORS_H
#define MULTI_TEMPERATURE_SENSORS_H

/*
  MultiTemperatureSensors.h
*/

#include "MultiTemperatureSensorDefines.h"

#include <Arduino.h>
#include "DallasTemperature.h"

class MultiTemperatureSensors
{
private:
	const float bad_temp = -127;
	const uint8_t no_mapping = 255;
	const uint8_t temperatur_precision = 9;

	DallasTemperature *sensors;

	DeviceAddress addresses[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];
	ssize_t address_params[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];
	uint8_t mappings[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];

	int timeout_convert = 1000;
	int timeout_repeat = 9000;

	bool is_repeat_timeout;
	unsigned long time_now = 0;

	uint8_t address_count;

	bool is_started = false;

	void mapAddresses();
	inline bool isAddressMatch(uint8_t address_index, uint8_t params_index)
	{
		return memcmp(((ssize_t *)(addresses + address_index)) + 1, address_params + params_index, sizeof(ssize_t)) == 0;
	}

	void (*temperatureChanged)(uint8_t, float);
	void (*addressChanged)(uint8_t, ssize_t);

public:
	MultiTemperatureSensors(
		DallasTemperature *oneWire,
		void (*temperatureChanged)(uint8_t, float),
		void (*addressChanged)(uint8_t, ssize_t));
	void start();
	void loop();
	void setRepeatTimeout(uint32_t value);
	void rescanAddresses();
	void setAddress(uint8_t index, uint32_t value);
	static const ssize_t min_address = 0x80000000;
	static const ssize_t max_address = 0x7fffffff;
};

#endif