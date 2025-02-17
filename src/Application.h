#ifndef _application_h_
#define _applicaiton_h_

#include "state_machine/States.h"
#include <FluxGarage_RoboEyes.h>
// #include <Adafruit_SSD1306.h>

class I2SSampler;
class I2SOutput;
class State;
class IndicatorLight;
class Buzzer;
class IntentProcessor;

class Application
{
private:
    State *m_detect_wake_word_state;
    State *m_recognise_command_state;
    State *m_current_state;

    roboEyes *m_eyes;
    Buzzer *m_buzzer;

public:
    Application(I2SSampler *sample_provider, IntentProcessor *intent_processor, Buzzer *buzzer, IndicatorLight *indicator_light, roboEyes *eyes);
    ~Application();
    void run();
};

#endif