#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS      7 
#define TFT_DC      3 
#define TFT_RST     10 
#define TFT_MOSI    6 
#define TFT_SCLK    4

#define POT_PIN     2   
#define LED_PIN     1  

// --- LOGO COLORS (RGB565) ---
#define BF_SKY_BLUE    0x867D  // Light blue sky
#define BF_DARK_BLUE   0x324D  // The shield outline/navy
#define BF_HILL_LIGHT  0x7E08  // Light green hill
#define BF_HILL_DARK   0x44C4  // Darker green hill
#define BF_BROWN       0x8200  // Tree trunk
#define BF_TEXT_GREY   0x4208  // Dark charcoal text
#define BLACK_BG    0x0000      // Black
#define GREEN_BG    0x2D43      // Forest Green

// STEVE SPECIFIC COLORS (RGB565)
#define STEVE_SKIN  0xFD4B // Pinkish Tan
#define STEVE_HAIR  0x4200 // Dark Brown
#define STEVE_EYE   0x421F // Blue/White mix
#define STEVE_MOUTH 0x6180 // Reddish Brown

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

uint32_t lastSerialPrint = 0; 
const int printInterval = 200; 

uint32_t lastHatToggle = 0;
const uint32_t hatInterval = 20000; 
bool hatIsOn = false;


void drawSteve(int x, int y, int s) {
    tft.startWrite();
    
    // 1. Skin Base
    tft.fillRect(x, y, 8*s, 8*s, STEVE_SKIN);
    
    // 2. Hair (Top 2 rows and sides)
    tft.fillRect(x, y, 8*s, 2*s, STEVE_HAIR); 
    tft.fillRect(x, y + 2*s, 1*s, 3*s, STEVE_HAIR);
    tft.fillRect(x + 7*s, y + 2*s, 1*s, 3*s, STEVE_HAIR);

    // 3. Eyes (White part then pupils)
    // Left eye
    tft.fillRect(x + 1*s, y + 4*s, 2*s, 1*s, 0xFFFF); 
    tft.fillRect(x + 1*s, y + 4*s, 1*s, 1*s, STEVE_EYE);
    // Right eye
    tft.fillRect(x + 5*s, y + 4*s, 2*s, 1*s, 0xFFFF);
    tft.fillRect(x + 6*s, y + 4*s, 1*s, 1*s, STEVE_EYE);

    // 4. Nose/Mouth area
    tft.fillRect(x + 3*s, y + 5*s, 2*s, 1*s, STEVE_MOUTH); // Nose
    tft.fillRect(x + 2*s, y + 6*s, 4*s, 1*s, STEVE_MOUTH); // Beard/Mouth

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

void setup() {
    Serial.begin(115200);
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(40000000); 
    tft.init(240, 320); 
    tft.setRotation(1);
    tft.fillScreen(GREEN_BG);

    drawSchoolLogo(20, 60); // Start with Elf
    
    pinMode(POT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    if (millis() - lastSerialPrint >= printInterval) {
        lastSerialPrint = millis();
        int potValue = analogRead(POT_PIN);
        int brightness = map(potValue, 0, 4095, 0, 255);
        analogWrite(LED_PIN, brightness);
    }

    if (millis() - lastHatToggle >= hatInterval) {
        lastHatToggle = millis();
        hatIsOn = !hatIsOn; 

        if (hatIsOn) {
            tft.fillScreen(GREEN_BG);
            drawSteve(100, 60, 15);
        } else {
            tft.fillScreen(GREEN_BG); // Same background as requested
            drawSchoolLogo(20, 60); 
        }
    }
}
