#ifndef CIRC_CONTROL_H
#define CIRC_CONTROL_H
/*
  CircControl.h
*/

#define CIRC_CONTROL_NUM_HEATER_TEMPERATURES 16

class CircControl
{
private:
    int timeout = 9000;
    unsigned long time_now = 0;

    float heaterTemperatures[CIRC_CONTROL_NUM_HEATER_TEMPERATURES];
    uint8_t heaterTemperatureIndex = 0;
    float mixerTemperature;
    float returnTemperature;
    void (*setOutput)(bool state);

public:
    CircControl(void (*setOutput)(bool state));
    void loop();
    void setHeaterTemperature(float temperature);
    void setMixerTemperature(float temperature);
    void setReturnTemperature(float temperature);
};

#endif