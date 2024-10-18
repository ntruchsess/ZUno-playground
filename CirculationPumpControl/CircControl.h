#ifndef CIRC_CONTROL_H
#define CIRC_CONTROL_H
/*
  CircControl.h
*/

#include <Arduino.h>

// #define CIRC_CONTROL_DEBUG

#define CIRC_CONTROL_NUM_HEATER_TEMPERATURES 16
#define CIRC_CONTROL_TEMPERATURE_INDEX_MASK 0x0f

#ifdef CIRC_CONTROL_DEBUG
#include <Stream.h>
#endif

class CircControl
{
private:
  unsigned long start_time = 0;
  bool pumpRunning = false;

  int16_t heaterTemp[CIRC_CONTROL_NUM_HEATER_TEMPERATURES];
  uint8_t heaterIndex = 0;
  int16_t mixerTemp;
  int16_t returnTemp;
  int minRunTime = 10000;
  int maxRunTime = 300000;
  int16_t filtered = 0;
  int16_t filterThreshold = 315;
  int16_t maxDifference = 50;

  uint8_t initialCounter = CIRC_CONTROL_NUM_HEATER_TEMPERATURES;

  void (&onPumpStateChanged)(bool state);
  void filter();

#ifdef CIRC_CONTROL_DEBUG
  Stream &debug;
#endif

public:
#ifdef CIRC_CONTROL_DEBUG
  CircControl(void (&onPumpStateChanged)(bool state), Stream &debug);
#else
  CircControl(void (&onPumpStateChanged)(bool state));
#endif

  void loop();
  void startPump();
  void stopPump();

  bool getPumpStatus() { return pumpRunning; };

  void setHeaterTemperature(int16_t temperature);
  void setMixerTemperature(int16_t temperature) { mixerTemp = temperature; };
  void setReturnTemperature(int16_t temperature) { returnTemp = temperature; };

  void setMinRunTime(uint16_t time) { minRunTime = time * 1000; };
  void setMaxRunTime(uint16_t time) { maxRunTime = time * 1000; };
  void setMaxTempDifference(int16_t difference) { maxDifference = difference; };
  void setFilterThreshold(int16_t threshold) { filterThreshold = threshold; };
};

#endif
