#ifndef _application_h_
#define _applicaiton_h_

#include "state_machine/States.h"
#include <FluxGarage_RoboEyes.h>
#include <Adafruit_SSD1306.h>

class I2SSampler;
class I2SOutput;
class State;
class IndicatorLight;
class Speaker;
class Buzzer;
class IntentProcessor;

class Application
{
private:
    State *m_detect_wake_word_state;
    State *m_recognise_command_state;
    State *m_current_state;

    Adafruit_SSD1306 *m_display;
    roboEyes *m_eyes;

public:
    Application(I2SSampler *sample_provider, IntentProcessor *intent_processor, Speaker *speaker, Buzzer *buzzer, IndicatorLight *indicator_light, Adafruit_SSD1306 *display);
    ~Application();
    void run();
};

#endif