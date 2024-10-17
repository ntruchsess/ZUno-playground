#ifndef MULTI_TEMPERATURE_SENSORS_H
#define MULTI_TEMPERATURE_SENSORS_H

/*
  MultiTemperatureSensors.h
*/

#include "MultiTemperatureSensorDefines.h"

#include <Arduino.h>
#include "ZUNO_OneWire.h"
#include "DallasTemperature.h"

class MultiTemperatureSensors
{
	private:
		const uint16_t bad_temp = -1270;
		const uint8_t no_mapping = 255;
		const uint8_t temperatur_precision = 9;

		uint8_t repeat_param;
		uint8_t rescan_param;
		uint8_t address_param_base;
		uint8_t base_channel;
		
		DallasTemperature sensors;

		DeviceAddress addresses[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];
		ssize_t address_params[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];
		uint8_t mappings[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];
		int16_t temperatures[MULTI_TEMPERATURE_SENSORS_MAX_SENSORS];

		int timeout_convert = 1000;
		int timeout_repeat = 9000;

		bool is_repeat_timeout;
		unsigned long time_now = 0;

		uint8_t address_count;

		void setRepeatTimeout(uint32_t value);
		void mapAddresses();
		void rescanAddresses();
		inline bool isAddressMatch(uint8_t address_index, uint8_t params_index)
		{
			return memcmp(((ssize_t *)(addresses + address_index)) + 1, address_params + params_index, sizeof(ssize_t)) == 0;
		}

	public:
		MultiTemperatureSensors(OneWire* oneWire, uint8_t baseChannel, uint8_t repeatParam, uint8_t rescanParam, uint8_t addressParams);
		void setup();
		void loop();
		void setParameter(uint8_t param_index, uint32_t value);
		int16_t getTemperature(uint8_t channel)
		{
			return temperatures[channel];
		}
		static const ssize_t min_address = 0x80000000;
		static const ssize_t max_address = 0x7fffffff;
};

#endif