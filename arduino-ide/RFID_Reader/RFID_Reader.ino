#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ------------------------------------
// 1. PIN DEFINITIONS
// ------------------------------------
#define TFT_CS 7
#define TFT_DC 3
#define TFT_RST 10
#define TFT_MOSI 6
#define TFT_SCLK 4

// ------------------------------------
// 2. GAUGE & COLOR DEFINITIONS (RGB565)
// ------------------------------------
#define GAUGE_RADIUS 45
#define GAUGE_X_OFFSET 80  // Center X for left column
#define GAUGE_Y_OFFSET 120  // Center Y for top row

#define BG_COLOR 0x0000     // Black
#define DIAL_COLOR 0x3186   // Dark Gray/Blue
#define NEEDLE_COLOR 0xF800 // Red
#define TEXT_COLOR 0x07E0   // White
#define ACCENT_COLOR 0x07E0 // Green

// ------------------------------------
// 3. GLOBAL VARIABLES
// ------------------------------------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Variables to track the last needle position (to erase it)
float lastAngle[4] = {0, 0, 0, 0}; 


// ------------------------------------
// 4. FUNCTION PROTOTYPES
// ------------------------------------
void drawGaugeOutline(int cx, int cy, int r, const char* label, const char* unit, float minVal, float maxVal);
void updateGauge(int index, int cx, int cy, int r, float value, float minVal, float maxVal, const char* unit);
float degreesToRadians(float degrees);


// ------------------------------------
// 5. SETUP
// ------------------------------------
void setup() {
    Serial.begin(115200);
    
    // Initialize SPI and Display
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(40000000); 
    tft.init(240, 320); 

    // Set Display Properties
    tft.setRotation(1); // Landscape (320x240)
    tft.fillScreen(BG_COLOR);
    
    // Draw all four static gauge outlines
    // Gauge 0: Temperature (Top Left)
    drawGaugeOutline(GAUGE_X_OFFSET, GAUGE_Y_OFFSET - 65, GAUGE_RADIUS, "TEMP", "C", 0, 100);
    
    // Gauge 1: Light (Bottom Left)
    drawGaugeOutline(GAUGE_X_OFFSET, GAUGE_Y_OFFSET + 55, GAUGE_RADIUS, "LIGHT", "%", 0, 100);
    
    // Gauge 2: Sound (Top Right)
    drawGaugeOutline(GAUGE_X_OFFSET + 160, GAUGE_Y_OFFSET - 65, GAUGE_RADIUS, "SOUND", "dB", 30, 100);
    
    // Gauge 3: Humidity (Bottom Right)
    drawGaugeOutline(GAUGE_X_OFFSET + 160, GAUGE_Y_OFFSET + 55, GAUGE_RADIUS, "HUMIDITY", "%", 0, 100);

    tft.setTextColor(TEXT_COLOR);
}


// ------------------------------------
// 6. LOOP (Simulation)
// ------------------------------------
void loop() {
    // --- SIMULATED SENSOR READINGS ---
    float temp = random(20, 30) + (float)random(0, 99) / 100.0;
    float light = random(50, 90);
    float sound = random(40, 70);
    float humidity = random(30, 60);

    // --- UPDATE GAUGE 0: TEMPERATURE ---
    updateGauge(0, GAUGE_X_OFFSET, GAUGE_Y_OFFSET - 70, GAUGE_RADIUS, temp, 0, 100, "C");
    
    // --- UPDATE GAUGE 1: LIGHT ---
    updateGauge(1, GAUGE_X_OFFSET, GAUGE_Y_OFFSET + 70, GAUGE_RADIUS, light, 0, 100, "%");
    
    // --- UPDATE GAUGE 2: SOUND ---
    updateGauge(2, GAUGE_X_OFFSET + 160, GAUGE_Y_OFFSET - 70, GAUGE_RADIUS, sound, 30, 100, "dB");
    
    // --- UPDATE GAUGE 3: HUMIDITY ---
    updateGauge(3, GAUGE_X_OFFSET + 160, GAUGE_Y_OFFSET + 70, GAUGE_RADIUS, humidity, 0, 100, "%");
    
    // Wait a moment before updating again
    delay(500); 
}


// ------------------------------------
// 7. GAUGE FUNCTION DEFINITIONS
// ------------------------------------

float degreesToRadians(float degrees) {
    return degrees * (PI / 180.0);
}

/**
 * Draws the static parts of the gauge (circle, marks, labels).
 */
void drawGaugeOutline(int cx, int cy, int r, const char* label, const char* unit, float minVal, float maxVal) {
    // Draw the main arc (240 degrees total, 120 to 360 degrees)
    tft.drawCircle(cx, cy, r, DIAL_COLOR);
    tft.drawCircle(cx, cy, r + 1, DIAL_COLOR); 

    // Draw Labels
    tft.setTextSize(1);
    tft.setTextColor(TEXT_COLOR);
    
    tft.setCursor(cx - (strlen(label) * 3), cy + r + 10);
    tft.print(label);
    
    // Draw Tick Marks and Min/Max labels
    for (int i = 0; i <= 6; i++) {
        float angle = 120 + (i * 40); // Angles from 120 to 360 (or -120 to 0)
        float rad = degreesToRadians(angle);
        
        int x1 = cx + (r * cos(rad));
        int y1 = cy + (r * sin(rad));
        int x2 = cx + ((r - 5) * cos(rad)); // Inner point for tick mark
        
        // Draw the tick mark
        tft.drawLine(x1, y1, x2, cy + ((r - 5) * sin(rad)), DIAL_COLOR);

        // Draw min/max numbers at the extremes
        if (i == 0) { // Min value
            tft.setCursor(x1 - 15, y1 - 10);
            tft.print((int)minVal);
        } else if (i == 6) { // Max value
            tft.setCursor(x1 + 5, y1 - 10);
            tft.print((int)maxVal);
        }
    }
}

/**
 * Draws the dynamic parts of the gauge (needle and digital value).
 */
void updateGauge(int index, int cx, int cy, int r, float value, float minVal, float maxVal, const char* unit) {
    // 1. Calculate the needle angle (Mapping value to 120 to 360 degrees)
    float normalizedVal = (value - minVal) / (maxVal - minVal);
    
    // Clamp the value just in case it's out of range
    if (normalizedVal < 0) normalizedVal = 0;
    if (normalizedVal > 1) normalizedVal = 1;

    // Angle starts at 120 degrees and sweeps 240 degrees (360 - 120)
    float currentAngle = 120 + (normalizedVal * 240);
    float currentRad = degreesToRadians(currentAngle);

    // Needle dimensions
    int needleLength = r - 10;
    int digitalY = cy + 10; // Position for digital reading

    // --- ERASE LAST NEEDLE POSITION ---
    // Calculate old needle end points
    float oldRad = degreesToRadians(lastAngle[index]);
    int oldX = cx + (needleLength * cos(oldRad));
    int oldY = cy + (needleLength * sin(oldRad));
    
    // Erase the old needle by drawing it in the background color
    tft.drawLine(cx, cy, oldX, oldY, BG_COLOR);

    // Erase the old digital reading (draw a rectangle over it)
    tft.fillRect(cx - 30, digitalY, 60, 18, BG_COLOR);

    // --- DRAW NEW NEEDLE POSITION ---
    int newX = cx + (needleLength * cos(currentRad));
    int newY = cy + (needleLength * sin(currentRad));
    
    // Draw the new needle in red
    tft.drawLine(cx, cy, newX, newY, NEEDLE_COLOR);
    tft.drawCircle(cx, cy, 3, NEEDLE_COLOR); // Center dot

    // --- DRAW DIGITAL READING ---
    tft.setTextSize(2);
    tft.setCursor(cx - 20, digitalY);
    
    // Print value with 1 decimal place, followed by unit
    char valueStr[10];
    snprintf(valueStr, sizeof(valueStr), "%.1f%s", value, unit);
    tft.print(valueStr);
    
    // Save the current angle for the next redraw
    lastAngle[index] = currentAngle;
}