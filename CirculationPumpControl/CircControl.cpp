#include "CircContol.h"
/*
  CircControl.cpp
*/


CircControl::CircControl(void (*setOutput)(bool state))
    : setOutput(setOutput)
{
}

void CircControl::setHeaterTemperature(float temperature)
{
    heaterTemperatures[heaterTemperatureIndex] = temperature;
    heaterTemperatureIndex++;
    if (heaterTemperatureIndex == CIRC_CONTROL_NUM_HEATER_TEMPERATURES)
    {
        heaterTemperatureIndex = 0;
    }
}

void CircControl::setMixerTemperature(float temperature)
{
    mixerTemperature = temperature;
}

void CircControl::setReturnTemperature(float temperature)
{
    returnTemperature = temperature;
}
