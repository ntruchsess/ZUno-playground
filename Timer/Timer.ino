/*
  Timer.ino
*/

enum
{
    TIMEOUT_PARAM = 64
};

ZUNO_SETUP_CONFIGPARAMETERS(
    ZUNO_CONFIG_PARAMETER_INFO("Timeout", "time to switch of in seconds", 0, 86400, 30));

ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);

int timeout_ms = 0;
unsigned long time_now = 0;
boolean switchState = false;

ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_BINARY(getSwitchValue, setSwitchValue));

void setup()
{
    setTimeout(zunoLoadCFGParam(TIMEOUT_PARAM));
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    if (switchState && (millis() - time_now > timeout_ms))
    {
        switchState = false;
        digitalWrite(LED_BUILTIN, LOW);
        zunoSendReport(1);
    }
}

void setSwitchValue(byte newValue)
{
    switchState = true;
    time_now = millis();
    digitalWrite(LED_BUILTIN, HIGH);
    zunoSendReport(1);
}

byte getSwitchValue(void)
{
    return switchState
               ? 255
               : 0;
}

void config_parameter_changed(uint8_t param, uint32_t value)
{
    if (param == TIMEOUT_PARAM)
    {
        setTimeout(value);
    }
}

void setTimeout(uint32_t value)
{
    timeout_ms = value * 1000;
    time_now = millis() - timeout_ms;
    switchState = false;
    zunoSendReport(1);
}
