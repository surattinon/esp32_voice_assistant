// WiFi credentials
#define WIFI_SSID "Bas"
#define WIFI_PSWD "basu086331"

// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define USE_I2S_MIC_INPUT

// I2S Microphone Settings

// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
/*#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT*/
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_33
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_25
#define I2S_MIC_SERIAL_DATA GPIO_NUM_32

// Analog Microphone Settings - ADC1_CHANNEL_7 is GPIO35
#define ADC_MIC_CHANNEL ADC1_CHANNEL_7

// speaker settings
#define I2S_SPEAKER_SERIAL_CLOCK GPIO_NUM_14
#define I2S_SPEAKER_LEFT_RIGHT_CLOCK GPIO_NUM_12
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_27

#define BUZZER_PIN GPIO_NUM_19

// command recognition settings
#define COMMAND_RECOGNITION_ACCESS_KEY "THU4ZC7HTNBACLTJLDUCWPODONWPC6SM"

// MQTT settings
#define MQTT_BROKER "172.20.10.10"
#define MQTT_PORT 1883
#define MQTT_USERNAME "ite233"
#define MQTT_PASSWORD "finalproject"
#define MQTT_CLIENT_ID "marvin1"

// ROBO EYES SETTINGS //

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

byte mood = 0; // Mood switch
byte position = 0; // Position switch

// Name aliases for different config mode states
#define EYES_WIDTHS 0
#define EYES_HEIGHTS 1
#define EYES_BORDERRADIUS 2
#define EYES_SPACEBETWEEN 3
#define CYCLOPS_TOGGLE 4
#define CURIOUS_TOGGLE 5
#define PREDEFINED_POSITIONS 6
byte configMode = 6; // for saving current config mode state
bool showConfigMode = 0; // for showing current config mode on display
unsigned long showConfigModeTimer = 0;
int showConfigModeDuration = 1500; // how long should the current config mode headline be displayed?