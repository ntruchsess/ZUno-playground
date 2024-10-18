#include "CircControl.h"
/*
  CircControl.cpp

  filter heater-temperatures, if filtered value exceeds threshold turn on pump.
  pump stops when it's runtime is greater then the configured minimum runtime
  and the difference of mixer-temp minus return-temp is smaller then the configured max difference
  or the maximum runtime is reached.
*/

#ifdef CIRC_CONTROL_DEBUG
CircControl::CircControl(
    void (&onPumpStateChanged)(bool state),
    Stream &debug)
    : onPumpStateChanged(onPumpStateChanged),
      debug(debug)
{
}
#else
CircControl::CircControl(
    void (&onPumpStateChanged)(bool state))
    : onPumpStateChanged(onPumpStateChanged)
{
}
#endif

void CircControl::loop()
{
  if (pumpRunning)
  {
    unsigned long now = millis();
    if (now - start_time > minRunTime && (mixerTemp - returnTemp < maxDifference || now - start_time > maxRunTime))
    {
      stopPump();
    }
  }
  else
  {
    if (filtered > filterThreshold)
    {
      startPump();
    }
  }
}

void CircControl::startPump()
{
  start_time = millis();
  pumpRunning = true;
  onPumpStateChanged(true);
}

void CircControl::stopPump()
{
  pumpRunning = false;
  onPumpStateChanged(false);
}

void CircControl::setHeaterTemperature(int16_t const temperature)
{
  ++heaterIndex &= CIRC_CONTROL_TEMPERATURE_INDEX_MASK;
  heaterTemp[heaterIndex] = temperature;
  if (initialCounter > 0)
  {
    initialCounter--;
  }
  else
  {
    filter();
  }
#ifdef CIRC_CONTROL_DEBUG
  debug.print("Heater: ");
  debug.print(heaterTemp[heaterIndex]);
  debug.print(", Mixer: ");
  debug.print(mixerTemp);
  debug.print(", Return: ");
  debug.print(returnTemp);
  debug.print(", Filter: ");
  debug.print(filtered);
  if (pumpRunning)
  {
    debug.print(", State: on, RunTime: ");
    debug.println(millis() - start_time);
  }
  else
  {
    debug.println(", State: off, RunTime: -");
  }
#endif
}

void CircControl::filter()
{
  const uint8_t fir_coefficients[CIRC_CONTROL_NUM_HEATER_TEMPERATURES] = {8, 14, 24, 40, 64, 96, 128, 128};
  uint8_t startIndex = (heaterIndex - (CIRC_CONTROL_NUM_HEATER_TEMPERATURES - 1)) & CIRC_CONTROL_TEMPERATURE_INDEX_MASK;
  int16_t sum = 0;
  for (uint8_t i = 0; i < CIRC_CONTROL_NUM_HEATER_TEMPERATURES / 2; i++)
  {
    sum += ((heaterTemp[(heaterIndex - i) & CIRC_CONTROL_TEMPERATURE_INDEX_MASK] - heaterTemp[(startIndex + i) & CIRC_CONTROL_TEMPERATURE_INDEX_MASK]) * fir_coefficients[i]) >> 3;
  }
  filtered = sum;
}
