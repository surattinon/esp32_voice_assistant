#ifndef _buzzer_h_
#define _buzzer_h_

class Buzzer
{
private:
    int m_pin;       // GPIO pin for buzzer
    bool m_isActive; // Current state of buzzer
    int m_frequency; // Current frequency

public:
    Buzzer(int pin); // Constructor taking GPIO pin number
    ~Buzzer();       // Destructor

    // Control methods
    void begin();                           // Initialize the buzzer
    void beep(int frequency, int duration); // Play a beep
    void playTone(int frequency);           // Play continuous tone
    void stop();                            // Stop the buzzer
    bool isActive();                        // Get current state

    // Optional: Add methods for different patterns
    void playSuccess();
    void playError();
    void playWake();
    void playListening();
};

#endif