#include "driver/ledc.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 


// --- CONFIGURATION DEFINES ---
#define THERMISTOR_PIN 3
#define SERIES_RESISTOR 10000.0 // Your 10kOhm resistor
#define NOMINAL_RESISTANCE 10000.0
#define NOMINAL_TEMPERATURE 25.0
#define BETA_COEFFICIENT 3950.0

#define LED_PIN 7
#define BUTTON_PIN 10

// --- BUZZER CONFIGURATION ---
#define BUZZER_PIN 10        // GPIO 8 used for Passive Buzzer (PWM)
#define BUZZER_CHANNEL 0    // ESP32 PWM channel (0-15)
#define BUZZER_RESOLUTION 8 // 8-bit resolution (0-255 duty cycle)
#define MAX_ADC_VALUE 4095.0
#define SUPPLY_VOLTAGE 3.3

// --- FIXED I2C PINS FOR ESP32-C3 ---
#define I2C_SDA_PIN 5
#define I2C_SCL_PIN 4
#define LDR_PIN 2
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C 
// ------------------------------------------

// --- MARIO THEME NOTES & CONSTANTS ---
// Frequencies for a few common notes
#define NOTE_C4 262
#define NOTE_G3 196
#define NOTE_E4 330
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_AS4 466
#define NOTE_A4S 466 // Alias for A#4
#define NOTE_G4 392
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698 // <-- F5 DEFINED
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_A5 880 // <-- A5 DEFINED
#define NOTE_REST 0 // Represents a pause

// Define note durations (these will be scaled)
#define WHOLE_NOTE 1000
#define HALF_NOTE 500
#define QUARTER_NOTE 250
#define EIGHTH_NOTE 125
#define SIXTEENTH_NOTE 62
// ------------------------------------------

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Passive Buzzer Functions (Unchanged) ---

void playTone(int freq, int duration) {
    if (freq == NOTE_REST) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    } else {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 128);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
    delay(duration);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}


void noTone() {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0); // Set duty cycle to 0 (OFF)
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

// --- UPDATED FUNCTION: Play Super Mario Bros. Theme ---
void playMarioTheme() {
    Serial.println(F("Playing Extended Mario Theme (Including Jump Triplet)!"));
    
    // Notes for the Super Mario Bros. Overworld Theme (Expanded)
    int melody[] = {
        // Phrase 1: E E E C E G (Bar 1-2)
        NOTE_E5, NOTE_E5, NOTE_REST, NOTE_E5, NOTE_REST, NOTE_C5, NOTE_E5, NOTE_G5,
        NOTE_REST, NOTE_REST, NOTE_G3, NOTE_REST,
        
        // Phrase 2: C G E A B A# A G (Bar 3-4)
        NOTE_C5, NOTE_REST, NOTE_G4, NOTE_REST, NOTE_E4, NOTE_REST, NOTE_A4, NOTE_REST,
        NOTE_B4, NOTE_REST, NOTE_A4S, NOTE_A4, NOTE_REST, NOTE_G4,
        
        // Phrase 3: E G A F D E (Bar 5-6, the fast run)
        NOTE_E5, NOTE_REST, NOTE_G5, NOTE_REST, NOTE_A5, NOTE_REST, NOTE_F5, NOTE_REST,
        NOTE_G5, NOTE_REST, NOTE_E5, NOTE_REST, NOTE_C5, NOTE_REST, NOTE_D5, NOTE_B4, 
        
        // Phrase 4: Jump Triplet + Quick Ascend (The "Jumpy Bit")
        NOTE_C5, NOTE_G4, NOTE_E4, // Triplet (Fast)
        NOTE_REST, NOTE_A4, NOTE_C5, NOTE_D5,
        NOTE_REST, NOTE_E5, NOTE_REST, NOTE_D5, NOTE_REST, NOTE_C5,
    };

    // Durations for the corresponding notes
    int durations[] = {
        // Phrase 1 
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,

        // Phrase 2 
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, QUARTER_NOTE,

        // Phrase 3 
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
        
        // Phrase 4 (Triplets need shorter duration, using SIXTEENTH)
        SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, // C G E (Triplet)
        SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE,
        EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE, EIGHTH_NOTE,
    };
    
    // Scale factor for the tempo. Adjust to speed up or slow down the whole tune.
    const float tempo = 1.35; 

    int notes = sizeof(melody) / sizeof(melody[0]);

    for (int i = 0; i < notes; i++) {
        // Calculate the duration for the note (90% play, 10% pause for separation)
        int noteDuration = (int)(durations[i] * tempo);
        int playDuration = (int)(noteDuration * 0.9);
        int pauseDuration = (int)(noteDuration * 0.1);

        playTone(melody[i], playDuration);
        delay(pauseDuration); // Small delay between notes (the rest part)
    }

    noTone();
}


// Function to convert ADC reading to temperature in Celsius (Unchanged)
float getThermistorTemp() {
// ... (Unchanged) ...
// 1. Read ADC valu (0-4095)
int adcValue = 0;
for (int i = 0; i < 10; i++) {
adcValue += analogRead(THERMISTOR_PIN);
delay(10);
}
adcValue /= 10;

// CRITICAL ADC CHECK
if (adcValue < 10) { 
Serial.println(F("ADC Reading too low! Check wiring or pin."));
return -999.0;
}

// 2. Calculate the Thermistor's Resistance (Rt)
double V_out = (adcValue / MAX_ADC_VALUE) * 3.3; 
double V_thermistor = SUPPLY_VOLTAGE - V_out;
double resistance = SERIES_RESISTOR * (V_thermistor / V_out);

// 3. Convert Resistance to Temperature using the Beta equation
double T0_K = NOMINAL_TEMPERATURE + 273.15;
double T_K = 1.0 / ( (1.0/T0_K) + (1.0/BETA_COEFFICIENT) * log(resistance / NOMINAL_RESISTANCE) );

// 4. Convert Kelvin to Celsius
float T_C = T_K - 273.15;
return T_C;
}

void setup() {
Serial.begin(115200);

// ESP32 ADC setup
analogReadResolution(12);
analogSetAttenuation(ADC_11db);

  // Set LED pin as output
pinMode(LED_PIN, OUTPUT);
pinMode(BUTTON_PIN, INPUT_PULLUP);
// I2C initialization
Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
Serial.println(F("SSD1306 allocation failed"));
for(;;);
}

  // --- Configure Buzzer PWM (Unchanged) ---
ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_8_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = 2000,
    .clk_cfg          = LEDC_AUTO_CLK
};

ledc_timer_config(&ledc_timer);

ledc_channel_config_t ledc_channel = {
    .gpio_num   = BUZZER_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel    = LEDC_CHANNEL_0,
    .intr_type  = LEDC_INTR_DISABLE,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

ledc_channel_config(&ledc_channel);


  display.clearDisplay(); 
Serial.println(F("SETUP COMPLETE. Starting loop..."));
}

void loop() {
// 1. Read sensor values
float tempC = getThermistorTemp();
int ldrRawValue = analogRead(LDR_PIN);
int buttonState = digitalRead(BUTTON_PIN);

if(tempC > 40){
  // 2. Tone Sequence (MARIO THEME!)
  playMarioTheme(); 
}

// 3. LED Control Logic
if (ldrRawValue < 1000) {
digitalWrite(LED_PIN, HIGH);
} else {
digitalWrite(LED_PIN, LOW);
}

// 4. Display Output
display.clearDisplay();
display.fillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT - 16, SSD1306_BLACK);
display.setTextSize(1); 
display.setTextColor(SSD1306_WHITE);
display.setCursor(0, 0);
display.print(F("LDR: "));
 display.print(ldrRawValue); 
display.setCursor(0, 16); 
display.print(F("Temp: "));
display.print(tempC, 1);
display.print(F(" C"));
display.display();

  // 5. Continuous Serial Logging
Serial.print(F("Temp: "));
Serial.print(tempC, 2);
  Serial.print(F(" C | LDR: "));
Serial.println(ldrRawValue); 
}