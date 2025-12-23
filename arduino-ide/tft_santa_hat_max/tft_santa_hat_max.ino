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

// COLOR DEFINITIONS
#define HAT_RED     0xF800  
#define BLACK_BG    0x0000  
#define GREEN_BG    0x2D43  
#define HAT_WHITE   0xFFFF  
#define ELF_GREEN   0x07E0 
#define SKIN_TONE   0xFF9D 
#define SHOE_BROWN  0x8200 

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

uint32_t lastSerialPrint = 0; 
const int printInterval = 200; 

uint32_t lastHatToggle = 0;
const uint32_t hatInterval = 20000; 
bool hatIsOn = false;

void setup() {
    Serial.begin(115200);
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(40000000); 
    tft.init(240, 320); 
    tft.setRotation(1);
    tft.fillScreen(BLACK_BG); 
    drawElf(60, 180); // Start with Elf
    
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
            drawSantaHat(160, 120, HAT_WHITE, GREEN_BG); 
        } else {
            tft.fillScreen(GREEN_BG); // Same background as requested
            drawElf(80, 180); 
        }
    }
}

void drawSantaHat(int x, int y, uint16_t hatColor, uint16_t bgColor) {
    int hatHeight = 100;
    int hatWidth = 120;
    int trimHeight = 20;
    tft.startWrite();
    tft.fillTriangle(x, y - hatHeight, x - hatWidth/2, y - trimHeight, x + hatWidth/2, y - trimHeight, HAT_RED);
    tft.fillRect(x - hatWidth/2, y - trimHeight, hatWidth, trimHeight, HAT_WHITE);
    tft.fillCircle(x, y - hatHeight, 10, HAT_WHITE);
    
    tft.setCursor(15, 180);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.println("Merry Christmas");
    tft.setCursor(120, 210);
    tft.println("Max");
    tft.endWrite();
}
void drawElf(int x, int y) {
    tft.startWrite();

    // 1. LEGS (Striped)
    tft.fillRect(x + 35, y + 40, 10, 30, HAT_WHITE);
    tft.fillRect(x + 55, y + 40, 10, 30, HAT_WHITE);
    tft.fillRect(x + 35, y + 50, 10, 5, HAT_RED); // Stripes
    tft.fillRect(x + 55, y + 50, 10, 5, HAT_RED);

    // 2. SHOES
    tft.fillCircle(x + 30, y + 70, 8, SHOE_BROWN);
    tft.fillCircle(x + 70, y + 70, 8, SHOE_BROWN);

    // 3. BODY (Green Tunic)
    tft.fillTriangle(x + 50, y - 20, x + 20, y + 40, x + 80, y + 40, ELF_GREEN);
    tft.fillRect(x + 40, y + 10, 20, 5, 0xCE60); // Yellow Belt

    // 4. ARMS
    tft.drawLine(x + 35, y + 10, x + 15, y + 30, ELF_GREEN);
    tft.drawLine(x + 65, y + 10, x + 85, y + 30, ELF_GREEN);
    tft.fillCircle(x + 15, y + 30, 4, SKIN_TONE); // Hands
    tft.fillCircle(x + 85, y + 30, 4, SKIN_TONE);

    // 5. HEAD
    tft.fillCircle(x + 50, y - 30, 25, SKIN_TONE);
    tft.fillTriangle(x + 25, y - 35, x + 15, y - 45, x + 25, y - 25, SKIN_TONE); // Left Ear
    tft.fillTriangle(x + 75, y - 35, x + 85, y - 45, x + 75, y - 25, SKIN_TONE); // Right Ear
    
    // Face details
    tft.drawPixel(x + 42, y - 35, ST77XX_BLACK); // Eyes
    tft.drawPixel(x + 58, y - 35, ST77XX_BLACK);
    tft.drawLine(x + 45, y - 25, x + 55, y - 25, ST77XX_BLACK); // Smile

    // 6. ELF HAT
    tft.fillTriangle(x + 50, y - 80, x + 25, y - 50, x + 75, y - 50, ELF_GREEN);
    tft.fillRect(x + 25, y - 53, 50, 8, HAT_RED); // Red trim
    tft.fillCircle(x + 50, y - 80, 6, HAT_WHITE); // Pompom

    // --- 7. NEW TEXT TO THE RIGHT ---
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    
    // Positioned to the right of the elf
    tft.setCursor(x + 110, y - 40); 
    tft.println("Merry");
    
    tft.setCursor(x + 110, y - 10);
    tft.println("Christmas");
    
    tft.setCursor(x + 110, y + 25);
    tft.setTextSize(4); // Make "Max" slightly bigger
    tft.println("Max");

    tft.endWrite();
}