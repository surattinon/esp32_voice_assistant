#include <Arduino.h>
#include "Buzzer.h"

Buzzer::Buzzer(int pin) {
    m_pin = pin;
    m_isActive = false;
    m_frequency = 0;
}

Buzzer::~Buzzer() {
    stop();
}

void Buzzer::begin() {
    pinMode(m_pin, OUTPUT);
    stop();
}

void Buzzer::beep(int frequency, int duration) {
    playTone(frequency);
    delay(duration);
    stop();
}

void Buzzer::playTone(int frequency) {
    m_frequency = frequency;
    m_isActive = true;
    tone(m_pin, frequency);
}

void Buzzer::stop() {
    m_isActive = false;
    noTone(m_pin);
}

bool Buzzer::isActive() {
    return m_isActive;
}

void Buzzer::playSuccess() {
    beep(784, 70);
    beep(880, 70);
    beep(1175, 70);
}

void Buzzer::playError() {
    beep(400, 150);
    beep(400, 70);
}

void Buzzer::playWake() {
    beep(1175, 70);
}