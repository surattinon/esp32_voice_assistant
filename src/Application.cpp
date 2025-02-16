#include <Arduino.h>
#include "Application.h"
#include "state_machine/DetectWakeWordState.h"
#include "state_machine/RecogniseCommandState.h"
#include "IndicatorLight.h"
#include "Speaker.h"
#include "Buzzer.h"
#include "IntentProcessor.h"

Application::Application(I2SSampler *sample_provider, IntentProcessor *intent_processor, Speaker *speaker, Buzzer *buzzer, IndicatorLight *indicator_light, Adafruit_SSD1306 *display)
{
    // detect wake word state - waits for the wake word to be detected
    m_detect_wake_word_state = new DetectWakeWordState(sample_provider);
    // command recongiser - streams audio to the server for recognition
    m_recognise_command_state = new RecogniseCommandState(sample_provider, indicator_light, speaker, buzzer, intent_processor);
    // start off in the detecting wakeword state
    m_current_state = m_detect_wake_word_state;
    m_current_state->enterState();

    m_display = display;
    m_eyes = new roboEyes(*display);
    m_eyes->begin(128, 64, 30);

    // detect wake word state - waits for the wake word to be detected
    m_detect_wake_word_state = new DetectWakeWordState(sample_provider);

    // command recognizer - streams audio to the server for recognition
    m_recognise_command_state = new RecogniseCommandState(sample_provider, indicator_light, speaker, buzzer, intent_processor);

    // start off in the detecting wakeword state
    m_current_state = m_detect_wake_word_state;
    m_current_state->enterState();

    // Set initial eyes mood
    m_eyes->setMood(EYES_DEFAULT);
    m_eyes->setIdleMode(true);
}

Application::~Application()
{
    delete m_eyes;
}

// process the next batch of samples
void Application::run()
{
    bool state_done = m_current_state->run();

    m_eyes->update();

    if (state_done)
    {
        m_current_state->exitState();
        // switch to the next state - very simple state machine so we just go to the other state...
        if (m_current_state == m_detect_wake_word_state)
        {
            m_current_state = m_recognise_command_state;
            m_eyes->anim_laugh(); // Play laugh animation when wake word detected
        }
        else
        {
            m_current_state = m_detect_wake_word_state;
            m_eyes->blink(); // Blink when returning to wake word detection
        }
        m_current_state->enterState();
    }
    vTaskDelay(10);
}
