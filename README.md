# ESP32 Voice Control

## Description
This project implements voice control on an ESP32 device using wake word detection and command recognition.

## Features
- Wake word detection
- Command recognition
- Indicator light and buzzer feedback
- Intent processing
- MQTT communication
- RoboEyes for visual feedback

## Usage
1. Ensure you have the necessary components set up:
   - ESP32 device
   - I2S microphone or analog microphone
   - Indicator light
   - Buzzer
   - MQTT broker for communication
   - RoboEyes for visual feedback on 128x64 oled display

2. Define your WiFi credentials and other configurations in `src/config.h`:
   ```cpp
   // WiFi credentials
   #define WIFI_SSID "YourWiFiSSID"
   #define WIFI_PSWD "YourWiFiPassword"

   // Define other settings like MQTT, sensors, etc.
   ```

3. Customize the wake word detection and command recognition by modifying the neural network models and processing in the respective state files:
   - `src/state_machine/DetectWakeWordState.cpp`
   - `src/state_machine/RecogniseCommandState.cpp`

4. Add your specific devices and GPIO pins for control in `src/main.cpp`:
   ```cpp
   // Example:
   intent_processor->addDevice("light", GPIO_NUM_19);
   intent_processor->addDevice("bedroom", GPIO_NUM_21);
   ```

5. Set up any additional configurations or customizations in the respective files as needed.

6. Compile and upload the code to your ESP32 device.

7. Power on the ESP32 device and follow these steps:
   - The device will start detecting the wake word.
   - Once the wake word is detected, the device will listen for commands.
   - Speak the command and wait for the response.
