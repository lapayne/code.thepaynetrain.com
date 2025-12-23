#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "max_image.h"

// --- Pin Definitions (From previous setup) ---
#define TFT_CS   7  
#define TFT_DC   3  
#define TFT_RST 10  
#define TFT_MOSI 6  
#define TFT_SCLK 4  

// Initialize the display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void drawFlashImage(const unsigned short *data, int x, int y, int width, int height);

// --- Dashboard Layout Constants ---
#define TITLE_COLOR ST77XX_YELLOW
#define VALUE_COLOR ST77XX_WHITE
#define BACKGROUND_COLOR ST77XX_BLACK
#define BORDER_COLOR ST77XX_BLUE

// Starting coordinates and height for each section
const int SECTION_HEIGHT = 70; // (320px height - 4 sections) / 4 = 80, using 70 for spacing
const int START_X = 10;
const int START_Y = 5;

// Global variables for dashboard data (simulated data)
float currentTemp = 25.5;
int currentHumidity = 60;
float currentPressure = 1012.3;
String currentStatus = "OK";


void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Display
  tft.init(240, 320); 
  tft.setRotation(1); 
  tft.fillScreen(ST77XX_BLACK);
  
  // 2. Draw the embedded image
  // You MUST use the correct width/height of the image you converted!
  const int IMG_W = 320; // Example: Replace with your image width
  const int IMG_H = 240; // Example: Replace with your image height
  
  // Draw the image at position (10, 10)
  drawFlashImage(my_logo_data, 10, 10, IMG_W, IMG_H); 
}

void loop() {
  // Your dashboard code here...
}

/**
 * Draws a 16-bit RGB565 color array from Flash/PROGMEM onto the TFT.
 * @param data: Pointer to the RGB565 array.
 * @param x, y: Top-left corner coordinates.
 * @param width, height: Dimensions of the image.
 */
void drawFlashImage(const unsigned short *data, int x, int y, int width, int height) {
    if (x >= tft.width() || y >= tft.height() || 
        (x + width) > tft.width() || (y + height) > tft.height()) {
        Serial.println("Image dimensions exceed screen bounds.");
        return;
    }

    tft.startWrite();
    tft.setAddrWindow(x, y, x + width - 1, y + height - 1);
    
    // The total number of pixels to draw
    size_t len = (size_t)width * height;
    
    // Push the entire array of 16-bit color values to the display
    // We use the SPI library's optimized bulk transfer for speed
    // This is the fastest method possible!
    SPI.transferBytes((uint8_t*)data, NULL, len * 2);

    tft.endWrite();
}
