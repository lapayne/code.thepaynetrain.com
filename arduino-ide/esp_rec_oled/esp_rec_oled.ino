#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <math.h>
#include "driver/ledc.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LittleFS.h>      
#include <ArduinoJson.h>   

// Structure and Variables
typedef struct struct_message {
  char uid[32];
} struct_message;

struct_message myData;
char displayName[32] = "";
char displayAllergy[64] = ""; // New buffer for allergy info
unsigned long lastUidTime = 0;        
const unsigned long displayTimeout = 5000; 
unsigned long lastSensorUpdate = 0;   

// Pin Definitions
const int tempPin = 1;
const int ldrPin = 2;
const int ledPin = 3;
const int buzzerPin = 4;
const int sdaPin = 6;  
const int sclPin = 5;  

// Constants
const float B_COEFFICIENT = 3950;
const float ROOM_TEMP_K = 298.15;
const float NOMINAL_RESISTANCE = 10000;
const float SERIES_RESISTOR = 10000;
const int LDR_THRESHOLD = 1000;
const float TEMP_ALARM = 40.0;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Audio Definitions (Shortened for brevity, keep your full list in your IDE)
#define NOTE_G3 196
#define NOTE_E4 330
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_A4S 466 
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698 
#define NOTE_G5 784
#define NOTE_A5 880 
#define NOTE_REST 0 
#define QUARTER_NOTE 250
#define EIGHTH_NOTE 125
#define SIXTEENTH_NOTE 62 

// --- Updated Lookup for Array Structure ---
void lookupTag(const char* id) {
  // Default values if not found
  strlcpy(displayName, id, sizeof(displayName));
  strlcpy(displayAllergy, "No Data", sizeof(displayAllergy));

  if (!LittleFS.exists("/tags.json")) return;

  File file = LittleFS.open("/tags.json", "r");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) return;

  // The JSON is an array [], so we iterate through it
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject item : arr) {
    if (strcmp(item["tagid"], id) == 0) {
      strlcpy(displayName, item["name"] | "Unknown", sizeof(displayName));
      strlcpy(displayAllergy, item["allergy"] | "None", sizeof(displayAllergy));
      break;
    }
  }
}

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  lookupTag(myData.uid);
  lastUidTime = millis(); 
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  if(!LittleFS.begin(true)) Serial.println("FS Failed");

  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Wire.begin(sdaPin, sclPin);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println(F("OLED failed"));
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  pinMode(ldrPin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Setup Buzzer LEDC
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_13_BIT,
      .timer_num = LEDC_TIMER_0,
      .freq_hz = 5000,
      .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&ledc_timer);
  ledc_channel_config_t ledc_channel = {
      .gpio_num = buzzerPin,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0
  };
  ledc_channel_config(&ledc_channel);

  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastSensorUpdate >= 500) {
    lastSensorUpdate = currentMillis;

    int rawLDR = analogRead(ldrPin);
    int rawTemp = analogRead(tempPin);

    float resistance = SERIES_RESISTOR * (4095.0 / (float)rawTemp - 1.0);
    float steinhart = log(resistance / NOMINAL_RESISTANCE) / B_COEFFICIENT;
    steinhart += 1.0 / ROOM_TEMP_K;
    float tempC = (1.0 / steinhart) - 273.15;

    digitalWrite(ledPin, (rawLDR < LDR_THRESHOLD) ? HIGH : LOW);

    display.clearDisplay();
    display.setCursor(0,0);

    if (currentMillis - lastUidTime < displayTimeout) {
      // --- TAG SCAN MODE ---
      display.setTextSize(1);
      display.println(displayName);   // Shows "Max P"
      display.println("------------");
      display.println(displayAllergy); // Shows "Gluten Free..."
    } 
    else {
      // --- SENSOR MODE ---
      display.setTextSize(1);
      display.print("Temp: "); display.print(tempC, 1); display.println(" C");
      display.print("LDR:  "); display.println(rawLDR);
    }
    display.display();

    if (tempC > TEMP_ALARM) {
       playMarioTheme(); 
    }
  }
}

// --- BUZZER FUNCTIONS ---
void noTone() {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void playTone(int frequency, int duration) {
  if (frequency == NOTE_REST) {
    noTone();
  } else {
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequency);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  }
  delay(duration);
  noTone();
}

void playMarioTheme() {
    int melody[] = {
        NOTE_E5, NOTE_E5, NOTE_REST, NOTE_E5, NOTE_REST, NOTE_C5, NOTE_E5, NOTE_G5,
        NOTE_REST, NOTE_REST, NOTE_G3, NOTE_REST,
        NOTE_C5, NOTE_REST, NOTE_G4, NOTE_REST, NOTE_E4, NOTE_REST, NOTE_A4, NOTE_REST,
        NOTE_B4, NOTE_REST, NOTE_A4S, NOTE_A4, NOTE_REST, NOTE_G4,
        NOTE_E5, NOTE_REST, NOTE_G5, NOTE_REST, NOTE_A5, NOTE_REST, NOTE_F5, NOTE_REST,
        NOTE_G5, NOTE_REST, NOTE_E5, NOTE_REST, NOTE_C5, NOTE_REST, NOTE_D5, NOTE_B4,
        NOTE_C5, NOTE_G4, NOTE_E4, NOTE_REST, NOTE_A4, NOTE_C5, NOTE_D5,
        NOTE_REST, NOTE_E5, NOTE_REST, NOTE_D5, NOTE_REST, NOTE_C5,
    };

    int durations[] = {
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, QUARTER_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
    };

    const float tempo = 1.35;
    int notes = sizeof(melody) / sizeof(melody[0]);

    for (int i = 0; i < notes; i++) {
        int noteDuration = (int)(durations[i] * tempo);
        playTone(melody[i], (int)(noteDuration * 0.9));
        delay((int)(noteDuration * 0.1));
    }
    noTone();
}