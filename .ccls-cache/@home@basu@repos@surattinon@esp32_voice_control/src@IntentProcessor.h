#ifndef _intent_processor_h_
#define _intent_processor_h_

#include <map>
#include "WitAiChunkedUploader.h"
#include <PubSubClient.h>

class Speaker;
class Buzzer;

enum IntentResult
{
    FAILED,
    SUCCESS,
    SILENT_SUCCESS // success but don't play ok sound
};

class IntentProcessor {
private:
    std::map<std::string, int> m_device_to_pin;
    IntentResult turnOnDevice(const Intent &intent);
    IntentResult tellJoke();
    IntentResult life();

    Speaker *m_speaker;
    Buzzer *m_buzzer;

    PubSubClient *mqtt_client;

public:
    IntentProcessor(Speaker *speaker, Buzzer *buzzer, PubSubClient *mqtt);
    void addDevice(const std::string &name, int gpio_pin);
    IntentResult processIntent(const Intent &intent);

    void publishMQTT(const std::string &topic, const std::string &payload)
    {

    if (mqtt_client->connected()) {

        bool success = mqtt_client->publish(topic.c_str(), payload.c_str());

        if (success) {

            Serial.println("MQTT message published successfully");

        } else {

            Serial.println("Failed to publish MQTT message");

        }

    } else {

        Serial.println("MQTT client not connected");

    }

}
};

#endif
