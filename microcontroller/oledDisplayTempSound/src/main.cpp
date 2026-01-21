#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 
#include <esp_now.h>
#include <WiFi.h>

// --- SENSOR CONFIGURATION ---
#define THERMISTOR_PIN 1
#define LDR_PIN 2
#define LED_PIN 3
#define SERIES_RESISTOR 10000.0 
#define NOMINAL_RESISTANCE 10000.0
#define NOMINAL_TEMPERATURE 25.0
#define BETA_COEFFICIENT 3950.0
#define MAX_ADC_VALUE 4095.0
#define SUPPLY_VOLTAGE 3.3

// --- BUZZER CONFIGURATION ---
#define BUZZER_PIN 4  
#define BUZZER_FREQ 5000
#define BUZZER_RES 8

// --- NOTE DURATIONS ---
#define QUARTER_NOTE 250
#define EIGHTH_NOTE 125
#define SIXTEENTH_NOTE 62

// --- I2C PINS ---
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C 

// --- MARIO THEME NOTES ---
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

typedef struct struct_message {
    char uid[32]; // Adjust size based on your expected UID length
} struct_message;

struct_message incomingData;

String lastUID = "";
unsigned long messageTimestamp = 0;
const unsigned long MESSAGE_DURATION = 5000; // 5 seconds

String defaultMessage = "SYSTEM STATUS";
String displayMessage = "WAITING...";



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);





// --- UPDATED BUZZER LOGIC (ARDUINO CORE 3.0+) ---
void playTone(int freq, int duration) {
    if (freq == NOTE_REST) {
        ledcWrite(BUZZER_PIN, 0); 
    } else {
        ledcWriteTone(BUZZER_PIN, freq);
    }
    delay(duration);
    ledcWrite(BUZZER_PIN, 0); // Stop sound
}

void noTone() {
    ledcWrite(BUZZER_PIN, 0);
}

void playMarioTheme() {
    int melody[] = {
        NOTE_E5, NOTE_E5, NOTE_REST, NOTE_E5, NOTE_REST, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_REST, NOTE_G3,
        NOTE_C5, NOTE_G4, NOTE_E4, NOTE_A4, NOTE_B4, NOTE_A4S, NOTE_A4, NOTE_G4
    };
    int durations[] = {
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, QUARTER_NOTE
    };

    for (int i = 0; i < sizeof(melody)/sizeof(melody[0]); i++) {
        playTone(melody[i], durations[i]);
        delay(durations[i] * 0.1); 
    }
    noTone();
}
float getThermistorTemp() {
    int adcValue = 0;
    for (int i = 0; i < 10; i++) {
        adcValue += analogRead(THERMISTOR_PIN);
        delay(5);
    }

    adcValue /= 10;

    if (adcValue < 10) return -999.0;
    double V_out = (adcValue / MAX_ADC_VALUE) * 3.3;
    double V_thermistor = SUPPLY_VOLTAGE - V_out;
    double resistance = SERIES_RESISTOR * (V_thermistor / V_out);
    double T_K = 1.0 / ( (1.0/(NOMINAL_TEMPERATURE + 273.15)) + (1.0/BETA_COEFFICIENT) * log(resistance / NOMINAL_RESISTANCE) );
    return T_K - 273.15;
}

// Callback function when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingByte, int len) {
    memcpy(&incomingData, incomingByte, sizeof(incomingData));

    String uid = String(incomingData.uid);
    uid.toUpperCase();

    Serial.print("Received UID: ");
    Serial.println(uid);

    messageTimestamp = millis(); // ⏱ start timer

    if (uid == "0496DA753E6180") {
        displayMessage = "ACCESS GRANTED";
        digitalWrite(LED_PIN, HIGH);
        playTone(NOTE_G5, 200);
    }
    else if (uid == "7F09D231") {
        displayMessage = "LIMITED ACCESS";
        playTone(NOTE_E5, 200);
    }
    else {
        displayMessage = "ACCESS DENIED";
        playTone(NOTE_G3, 400);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Setup WiFi for ESP-NOW
    WiFi.mode(WIFI_STA);
    Serial.print("My MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

    pinMode(LED_PIN, OUTPUT);
    ledcAttach(BUZZER_PIN, BUZZER_FREQ, BUZZER_RES);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        for(;;); 
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 10);
    display.println("READY & WAITING...");
    display.display();
}

void loop() {
    float temp = getThermistorTemp();
    int light = analogRead(LDR_PIN);

    digitalWrite(LED_PIN, (light < 1000) ? HIGH : LOW);

    display.clearDisplay();
   
   // Revert message after 5 seconds
if (displayMessage != defaultMessage &&
    millis() - messageTimestamp > MESSAGE_DURATION) {
    displayMessage = defaultMessage;
    digitalWrite(LED_PIN, LOW);
}
display.clearDisplay();
display.setCursor(0, 0);
display.print(displayMessage);
display.drawFastHLine(0, 9, 128, SSD1306_WHITE);



    display.setCursor(0, 14);
    display.printf("Temp: %.1f C", temp);

    display.setCursor(0, 24);
    display.printf("Light: %d", light);
    display.display();

    if(temp > 40.0) playMarioTheme();

    delay(500);
}