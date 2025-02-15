#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <driver/i2s.h>
#include <esp_task_wdt.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>
#include "I2SMicSampler.h"
#include "ADCSampler.h"
#include "I2SOutput.h"
#include "config.h"
#include "Application.h"
#include "SPIFFS.h"
#include "IntentProcessor.h"
#include "Speaker.h"
#include "Buzzer.h" 
#include "IndicatorLight.h"
#include "RoboEyes.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

byte configMode = 6; // for saving current config mode state
byte mood = 0; // Mood switch
byte position = 0; // Position switch
bool showConfigMode = 0; // for showing current config mode on display
unsigned long showConfigModeTimer = 0;
int showConfigModeDuration = 1500; // how long should the current config mode headline be displayed?

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
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
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s microphone pins
i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

// i2s speaker pins
i2s_pin_config_t i2s_speaker_pins = {
    .bck_io_num = I2S_SPEAKER_SERIAL_CLOCK,
    .ws_io_num = I2S_SPEAKER_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_SPEAKER_SERIAL_DATA,
    .data_in_num = I2S_PIN_NO_CHANGE};

// This task does all the heavy lifting for our application
void applicationTask(void *param)
{
  Application *application = static_cast<Application *>(param);

  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true)
  {
    // wait for some audio samples to arrive
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0)
    {
      application->run();
    }
  }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {}

void setup_mqtt()
{

  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(mqtt_callback);
  while (!mqtt_client.connected())
  {

    Serial.println("Connecting to MQTT...");

    if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("MQTT connected");
    }
    else
    {

      Serial.print("Failed, rc=");
      Serial.println(mqtt_client.state());
      delay(2000);
    }
  }
}

void RoboEyesSetup() {

  // Startup robo eyes
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100); // screen-width, screen-height, max framerate
  roboEyes.close(); // start with closed eyes 
  roboEyes.setPosition(DEFAULT); // eyes should be initially centered

  // Define eyes behaviour for demonstration
  roboEyes.setAutoblinker(ROBO_ON, 3, 2); // Start auto blinker animation cycle -> bool active, int interval, int variation -> turn on/off, set interval between each blink in full seconds, set range for random interval variation in full seconds
  roboEyes.setIdleMode(ROBO_ON, 3, 1); // Start idle animation cycle (eyes looking in random directions) -> set interval between each eye repositioning in full seconds, set range for random time interval variation in full seconds

  //display.invertDisplay(true); // show inverted display colors (black eyes on bright background)

}

void RoboEyes() {
  if (!showConfigMode){
    roboEyes.update();  // Updates eye drawings limited by max framerate (good for fast controllers to limit animation speed). 
                        // If you want to use the full horsepower of your controller without limits, you can use drawEyes(); instead.
  } else {
    // Show current config mode headline in display
    display.clearDisplay(); // clear screen
    // Basic text setup for displaying config mode headlines
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,3);
    if(configMode == EYES_WIDTHS){
      display.println("Widths"); 
      display.println(roboEyes.eyeLwidthCurrent);
      }
    else if(configMode == EYES_HEIGHTS){
      display.println("Heights"); 
      display.println(roboEyes.eyeLheightCurrent);
      }
    else if(configMode == EYES_BORDERRADIUS){
      display.println("Border \nRadius"); 
      display.println(roboEyes.eyeLborderRadiusCurrent);
      }
    else if(configMode == EYES_SPACEBETWEEN){
      display.println("Space \nBetween"); 
      display.println(roboEyes.spaceBetweenCurrent);
      }
    else if(configMode == CYCLOPS_TOGGLE){
      display.println("Cyclops \nToggle");
      }
    else if(configMode == CURIOUS_TOGGLE){
      display.println("Curiosity \nToggle");
      }
    else if(configMode == PREDEFINED_POSITIONS){
      display.println("Predefined\nPositions"); 
      roboEyes.setIdleMode(0); // turn off idle mode
      roboEyes.setPosition(DEFAULT); // start with middle centered eyes
      }
    display.display(); // additionally show configMode on display
    if(millis() >= showConfigModeTimer+showConfigModeDuration){
      showConfigMode = 0; // don't show the current config mode on the screen anymore
    }
  }
}


void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting up");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C or 0x3D
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  RoboEyesSetup();
  // start up wifi
  // launch WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  setup_mqtt();
  Serial.printf("Total heap: %d\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

  // startup SPIFFS for the wav files
  SPIFFS.begin();
  // make sure we don't get killed for our long running tasks
  esp_task_wdt_init(10, false);

  // start up the I2S input (from either an I2S microphone or Analogue microphone via the ADC)
#ifdef USE_I2S_MIC_INPUT
  // Direct i2s input from INMP441 or the SPH0645
  I2SSampler *i2s_sampler = new I2SMicSampler(i2s_mic_pins, false);
#else
  // Use the internal ADC
  I2SSampler *i2s_sampler = new ADCSampler(ADC_UNIT_1, ADC_MIC_CHANNEL);
#endif

  // start the i2s speaker output
  I2SOutput *i2s_output = new I2SOutput();
  i2s_output->start(I2S_NUM_1, i2s_speaker_pins);
  Speaker *speaker = new Speaker(i2s_output);
  Buzzer *buzzer = new Buzzer(BUZZER_PIN);

  // indicator light to show when we are listening
  IndicatorLight *indicator_light = new IndicatorLight();

  // and the intent processor
  IntentProcessor *intent_processor = new IntentProcessor(speaker, buzzer, &mqtt_client);

  // intent_processor->addDevice("light", GPIO_NUM_19);
  // intent_processor->addDevice("bedroom", GPIO_NUM_21);
  // intent_processor->addDevice("table", GPIO_NUM_23);

  // create our application
  Application *application = new Application(i2s_sampler, intent_processor, speaker, buzzer, indicator_light);

  // set up the i2s sample writer task
  TaskHandle_t applicationTaskHandle;
  xTaskCreate(applicationTask, "Application Task", 8192, application, 1, &applicationTaskHandle);

  // start sampling from i2s device - use I2S_NUM_0 as that's the one that supports the internal ADC
#ifdef USE_I2S_MIC_INPUT
  i2s_sampler->start(I2S_NUM_0, i2sMemsConfigBothChannels, applicationTaskHandle);
#else
  i2s_sampler->start(I2S_NUM_0, adcI2SConfig, applicationTaskHandle);
#endif
}

void loop()
{
  vTaskDelay(50);
  RoboEyes();
  mqtt_client.loop();
}
