#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ------------------------------------
// 1. PIN DEFINITIONS
// ------------------------------------
#define TFT_CS      7 
#define TFT_DC      3 
#define TFT_RST     10 
#define TFT_MOSI    6 
#define TFT_SCLK    4

#define POT_PIN     2   // Potentiometer input
#define LED_PIN     1  // Onboard LED on ESP32-C3 SuperMini

// ------------------------------------
// 2. COLOR DEFINITIONS (RGB565)
// ------------------------------------
#define HAT_RED     0xF800      // Bright Red
#define BLACK_BG    0x0000      // Black
#define GREEN_BG    0x2D43      // Forest Green
#define HAT_WHITE   0xFFFF      // White
#define HAT_ERASE   0x0000      // Erase color


// --- LOGO COLORS (RGB565) ---
#define BF_SKY_BLUE    0x867D  // Light blue sky
#define BF_DARK_BLUE   0x324D  // The shield outline/navy
#define BF_HILL_LIGHT  0x7E08  // Light green hill
#define BF_HILL_DARK   0x44C4  // Darker green hill
#define BF_BROWN       0x8200  // Tree trunk
#define BF_TEXT_GREY   0x4208  // Dark charcoal text

// ------------------------------------
// 3. GLOBAL VARIABLES
// ------------------------------------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// TIMING VARIABLES
uint32_t lastSerialPrint = 0; 
const int printInterval = 200; // Read Pot & Update LED every 200ms

uint32_t lastHatToggle = 0;
const uint32_t hatInterval = 20000; // Update Screen every 20 seconds
bool hatIsOn = false;

void setup() {
    Serial.begin(115200);
    
    // Initialize SPI and Display
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(40000000); 
    tft.init(240, 320); 
    tft.setRotation(1);
    tft.fillScreen(BLACK_BG); 
    drawSchoolLogo(20, 40);  
    
    // Configure Pins
    pinMode(POT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    // --- TIMER 1: POTENTIOMETER & BRIGHTNESS (Every 200ms) ---
    if (millis() - lastSerialPrint >= printInterval) {
        lastSerialPrint = millis();
        int potValue = analogRead(POT_PIN);
        int brightness = map(potValue, 0, 4095, 0, 255);
        analogWrite(LED_PIN, brightness);
    }

    // --- TIMER 2: IMAGE TOGGLE (Every 100 Seconds) ---
    if (millis() - lastHatToggle >= hatInterval) {
        lastHatToggle = millis();
        hatIsOn = !hatIsOn; 

        if (hatIsOn) {
            // Screen 1: Elf Hat on Green
            tft.fillScreen(GREEN_BG);
            drawSantaHat(160, 120, HAT_WHITE, GREEN_BG); 
        } else {
            // Screen 2: School Logo on White
            tft.fillScreen(BLACK_BG); // White background matches logo better
            drawSchoolLogo(20, 40);    // Centered vertically
        }
    }
}

// ------------------------------------
// 4. DRAWING FUNCTION
// ------------------------------------
void drawSantaHat(int x, int y, uint16_t hatColor, uint16_t bgColor) {
    int hatHeight = 100;
    int hatWidth = 120;
    int trimHeight = 20;
    uint16_t trimPompomColor = HAT_WHITE;
    uint16_t primaryColor = (bgColor == BLACK_BG) ? HAT_ERASE : HAT_RED;
    uint16_t trimColor = (bgColor == BLACK_BG) ? HAT_ERASE : trimPompomColor;

    tft.startWrite();
    tft.fillTriangle(x, y - hatHeight, x - hatWidth/2, y - trimHeight, x + hatWidth/2, y - trimHeight, primaryColor);
    tft.fillRect(x - hatWidth/2, y - trimHeight, hatWidth, trimHeight, trimColor);
    tft.fillCircle(x, y - hatHeight, 10, trimColor);
    
    tft.setCursor(15, 180);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.println("Merry Christmas");
    tft.setCursor(120, 210);
    tft.println("Max");
    tft.endWrite();
}




void drawSchoolLogo(int x, int y) {
    tft.startWrite();

    // 1. THE SHIELD BACKGROUND
    tft.fillRect(x, y, 120, 100, BF_SKY_BLUE);
    tft.fillTriangle(x, y + 100, x + 120, y + 100, x + 60, y + 150, BF_SKY_BLUE);
    
    // 2. THE HILLS (Using fillCircle to simulate the green ground)
    tft.fillCircle(x + 30, y + 115, 45, BF_HILL_LIGHT);
    tft.fillCircle(x + 90, y + 105, 40, BF_HILL_LIGHT);

    // 3. THE TREE
    tft.fillRect(x + 58, y + 55, 6, 20, BF_BROWN); // Trunk
    tft.fillCircle(x + 60, y + 45, 18, BF_HILL_DARK); // Leaves
    // Red Apples
    tft.fillCircle(x + 52, y + 42, 3, 0xF800); // HAT_RED hex
    tft.fillCircle(x + 68, y + 48, 3, 0xF800);
    tft.fillCircle(x + 60, y + 35, 3, 0xF800);

    // 4. THE SHIELD BORDER (Outline)
    tft.drawRect(x, y, 120, 100, BF_DARK_BLUE);
    tft.drawLine(x, y + 100, x + 60, y + 150, BF_DARK_BLUE);
    tft.drawLine(x + 120, y + 100, x + 60, y + 150, BF_DARK_BLUE);
    
    // 5. DECORATIVE HILL LINES (Using drawCircleHelper for arcs)
    // drawCircleHelper(x, y, radius, corner_index, color)
    // corners: 1=top-right, 2=top-left, 4=bottom-left, 8=bottom-right
    tft.drawCircleHelper(x + 40, y + 110, 30, 2, BF_HILL_DARK); 
    tft.drawCircleHelper(x + 80, y + 100, 35, 1, BF_HILL_DARK);

    // 6. THE TEXT
    tft.setFont(NULL); 
    tft.setTextColor(BF_TEXT_GREY);
    
    tft.setTextSize(3);
    tft.setCursor(x + 140, y + 30);
    tft.print("Bridge");
    tft.setCursor(x + 140, y + 65);
    tft.print("Farm");

    tft.endWrite();
}