#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <FluxGarage_RoboEyes.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <esp_task_wdt.h>

#include "Application.h"
#include "Buzzer.h"
#include "FluxGarage_RoboEyes.h"
#include "I2SMicSampler.h"
#include "IndicatorLight.h"
#include "IntentProcessor.h"
#include "SPIFFS.h"
#include "config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
roboEyes eyes(display);

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// PM and PIR Sensor setup
byte buff[2];
unsigned long durationPM10;
unsigned long durationPM25;
unsigned long starttime;
unsigned long endtime;
unsigned long sampleTime = 30000;
unsigned long lowpulseoccupancyPM10 = 0;
unsigned long lowpulseoccupancyPM25 = 0;

int pir = 0;
int i = 0;

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s config for reading from both channels of I2S
i2s_config_t i2sMemsConfigBothChannels = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_MIC_CHANNEL,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s microphone pins
i2s_pin_config_t i2s_mic_pins = {.bck_io_num = I2S_MIC_SERIAL_CLOCK,
                                 .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
                                 .data_out_num = I2S_PIN_NO_CHANGE,
                                 .data_in_num = I2S_MIC_SERIAL_DATA};

// This task does all the heavy lifting for our application
void applicationTask(void *param) {
  Application *application = static_cast<Application *>(param);

  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true) {
    // wait for some audio samples to arrive
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0) {
      application->run();
    }
  }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {}

void setup_mqtt() {

  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(mqtt_callback);
  while (!mqtt_client.connected()) {

    Serial.println("Connecting to MQTT...");

    if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("MQTT connected");
    } else {

      Serial.print("Failed, rc=");
      Serial.println(mqtt_client.state());
      delay(2000);
    }
  }
}

void roboEyesSetup() {
  eyes.begin(128, 64, 30);
  eyes.setIdleMode(EYES_ON, 2, 5);
  eyes.setAutoblinker(EYES_ON, 2, 4);
}

void sensorSetup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(PM25PIN, INPUT);
}

void publishMQTT(const std::string &topic, const std::string &payload) {
  if (mqtt_client.connected()) {
    bool success = mqtt_client.publish(topic.c_str(), payload.c_str());
    if (success) {
      Serial.println("MQTT message published successfully");
    } else {
      Serial.println("Failed to publish MQTT message");
    }
  } else {
    Serial.println("MQTT client not connected");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  roboEyesSetup();

  Serial.println("Starting up");
  // start up wifi
  // launch WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  setup_mqtt();

  sensorSetup();

  Serial.printf("Total heap: %d\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

  // startup SPIFFS for the wav files
  SPIFFS.begin();
  // make sure we don't get killed for our long running tasks
  esp_task_wdt_init(10, false);

  // start up the I2S input (from either an I2S microphone or Analogue
  // microphone via the ADC)
#ifdef USE_I2S_MIC_INPUT
  // Direct i2s input from INMP441 or the SPH0645
  I2SSampler *i2s_sampler = new I2SMicSampler(i2s_mic_pins, false);
#else
  // Use the internal ADC
  I2SSampler *i2s_sampler = new ADCSampler(ADC_UNIT_1, ADC_MIC_CHANNEL);
#endif

  Buzzer *buzzer = new Buzzer(BUZZER_PIN);

  // indicator light to show when we are listening
  IndicatorLight *indicator_light = new IndicatorLight();

  // and the intent processor
  IntentProcessor *intent_processor = new IntentProcessor(buzzer, &mqtt_client);

  // intent_processor->addDevice("light", GPIO_NUM_19);
  // intent_processor->addDevice("bedroom", GPIO_NUM_21);
  // intent_processor->addDevice("table", GPIO_NUM_23);

  // create our application
  Application *application = new Application(i2s_sampler, intent_processor,
                                             buzzer, indicator_light, &eyes);

  // set up the i2s sample writer task
  TaskHandle_t applicationTaskHandle;
  xTaskCreate(applicationTask, "Application Task", 8192, application, 1,
              &applicationTaskHandle);

  // start sampling from i2s device - use I2S_NUM_0 as that's the one that
  // supports the internal ADC
#ifdef USE_I2S_MIC_INPUT
  i2s_sampler->start(I2S_NUM_0, i2sMemsConfigBothChannels,
                     applicationTaskHandle);
#else
  i2s_sampler->start(I2S_NUM_0, adcI2SConfig, applicationTaskHandle);
#endif
  starttime = millis();
}

float calculateConcentration(long lowpulseInMicroSeconds,
                             long durationinSeconds) {
  float ratio = (lowpulseInMicroSeconds / 1000000.0) / 30.0 *
                100.0; // Calculate the ratio
  float concentration = 0.001915 * pow(ratio, 2) + 0.09522 * ratio -
                        0.04884; // Calculate the mg/m3
  Serial.print("lowpulseoccupancy:");
  Serial.print(lowpulseInMicroSeconds);
  Serial.print("    ratio:");
  Serial.print(ratio);
  Serial.print("    Concentration:");
  Serial.println(concentration);
  return concentration;
}

void loop() {
  mqtt_client.loop();
  eyes.update();

  pir = digitalRead(PIR_PIN);
  durationPM25 = pulseIn(PM25PIN, LOW, 1000);
  lowpulseoccupancyPM25 += durationPM25;

  endtime = millis();

  // Publish motion status immediately when detected
  static int lastPirState = -1;
  if (pir != lastPirState) {
    lastPirState = pir;
    publishMQTT(TOPIC_MOTION, pir == HIGH ? "1" : "0");
  }

  if ((endtime - starttime) >= sampleTime) {

    float conPM25 = calculateConcentration(lowpulseoccupancyPM25, 30);
    float value = fabs(conPM25);
    float ugm3 = value * 1000;
    Serial.printf("PM2.5: %f ug/m3\n", ugm3);

    // Publish PM values to MQTT
    publishMQTT(TOPIC_PM25, std::to_string(ugm3));

    lowpulseoccupancyPM25 = 0;
    starttime = millis();
  }

  vTaskDelay(1);
}
