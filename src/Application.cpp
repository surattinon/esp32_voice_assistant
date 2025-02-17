#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Application.h"
#include "state_machine/DetectWakeWordState.h"
#include "state_machine/RecogniseCommandState.h"
#include "IndicatorLight.h"
#include "Buzzer.h"
#include "IntentProcessor.h"

Application::Application(I2SSampler *sample_provider, IntentProcessor *intent_processor, Buzzer *buzzer, IndicatorLight *indicator_light, roboEyes *eyes)
{
    // detect wake word state - waits for the wake word to be detected
    m_detect_wake_word_state = new DetectWakeWordState(sample_provider);
    // command recongiser - streams audio to the server for recognition
    m_recognise_command_state = new RecogniseCommandState(sample_provider, indicator_light, buzzer, intent_processor, eyes);
    // start off in the detecting wakeword state
    m_current_state = m_detect_wake_word_state;
    m_current_state->enterState();

    m_buzzer = buzzer;
    m_eyes = eyes;
}

Application::~Application()
{
    delete m_eyes;
}

void playListeningSound(void *param)
{
    Buzzer *buzzer = static_cast<Buzzer *>(param);
    buzzer->playListening();
    vTaskDelete(NULL); // Delete the task once done
}

// process the next batch of samples
void Application::run()
{

    bool state_done = m_current_state->run();
    // Update eyes based on current state

    if (m_current_state == m_detect_wake_word_state)
    {
        m_eyes->setMood(EYES_DEFAULT);
        m_eyes->setIdleMode(EYES_ON, 3, 6);
        m_eyes->setAutoblinker(EYES_ON, 4, 6);
    }

    if (state_done)
    {
        m_current_state->exitState();
        // switch to the next state - very simple state machine so we just go to the other state...
        if (m_current_state == m_detect_wake_word_state)
        {
            m_current_state = m_recognise_command_state;
        }
        else
        {
            m_current_state = m_detect_wake_word_state;
            m_eyes->setMood(EYES_HAPPY);
        }
        m_current_state->enterState();
    }

    vTaskDelay(10);
}
