#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>      // Filesystem base library
#include <LittleFS.h> // LittleFS library for file system access

// --- Configuration Settings ---
// !!! CRITICAL: UPDATE THESE WITH YOUR NETWORK DETAILS !!!
const char* ssid = "Home";
const char* password = "Tinker11!!";

// Define the GPIO pin for the onboard LED on the ESP32-C3 (usually pin 8)
const int ledPin = 8;
const unsigned long BLINK_INTERVAL_MS = 500; // 500ms on, 500ms off

// Create a WebServer object on port 80 (standard HTTP port)
WebServer server(80);

// Global state variables
// 0: OFF, 1: ON (Steady), 2: BLINKING
int ledMode = 0; 
// Stores the digital state of the pin (LOW/HIGH)
int ledPinState = HIGH; 
// Timing variable for blinking logic
unsigned long previousMillis = 0;


// --- Helper Functions ---

// Determines the correct MIME type for the browser when serving a file
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".jpg")) return "image/jpeg";
  if (filename.endsWith(".ico")) return "image/x-icon";
  if (filename.endsWith(".json")) return "application/json";
  if (filename.endsWith(".woff2")) return "font/woff2"; 
  return "text/plain";
}

// Function to serve files from LittleFS
bool serveStaticFile(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }

  if (LittleFS.exists(path)) {
    String contentType = getContentType(path);
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true; 
  }
  return false; 
}

// --- Handler Functions (API Endpoints for React Frontend) ---

// Handles the status check. Returns the high-level operating mode.
void handleStatus() {
  // Return ledMode: 0 (OFF), 1 (ON), or 2 (BLINKING)
  server.send(200, "text/plain", String(ledMode));
}

// Handles the command to turn the LED ON
void handleLedOn() {
  ledMode = 1; // Set mode to ON
  ledPinState = LOW; // LOW is ON for C3's Active-Low onboard LED
  digitalWrite(ledPin, ledPinState);
  server.send(200, "text/plain", "LED ON");
}

// Handles the command to turn the LED OFF
void handleLedOff() {
  ledMode = 0; // Set mode to OFF
  ledPinState = HIGH; // HIGH is OFF
  digitalWrite(ledPin, ledPinState);
  server.send(200, "text/plain", "LED OFF");
}

// Handles the command to start blinking
void handleLedBlink() {
  ledMode = 2; // Set mode to BLINKING
  // Start the blink sequence (sets the initial state and timer)
  ledPinState = LOW; // Start ON
  digitalWrite(ledPin, ledPinState);
  previousMillis = millis();
  server.send(200, "text/plain", "LED BLINKING");
}

// --- Catch-All File Server ---

void handleNotFound() {
  String path = server.uri();
  
  if (serveStaticFile(path)) {
    return;
  }
  
  server.send(404, "text/plain", "404: File or API endpoint not found.");
}


// --- Setup and Loop ---

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  // Initialize LED to OFF (HIGH)
  digitalWrite(ledPin, ledPinState); 
  
  // --- INITIALIZE LITTLEFS ---
  Serial.print("Initializing LittleFS...");
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed!");
    return;
  }
  Serial.println("LittleFS Mount Successful. File system ready.");
  // --- END LITTLEFS INIT ---

  // Connect to the Wi-Fi 
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected successfully!");
  Serial.print("Server IP address: ");
  Serial.println(WiFi.localIP());

  // --- Configure Web Server Routes ---
  server.on("/STATUS", handleStatus);
  server.on("/LED_ON", handleLedOn); 
  server.on("/LED_OFF", handleLedOff); 
  // NEW ENDPOINT
  server.on("/LED_BLINK", handleLedBlink); 
  
  server.onNotFound(handleNotFound); 

  server.begin();
  Serial.println("HTTP server started on port 80.");
}

void loop() {
  server.handleClient();
  
  // --- BLINKING LOGIC ---
  if (ledMode == 2) {
    unsigned long currentMillis = millis();
    
    // Check if the interval has passed
    if (currentMillis - previousMillis >= BLINK_INTERVAL_MS) {
      previousMillis = currentMillis; // Reset the timer
      
      // Toggle the LED state (LOW <-> HIGH)
      if (ledPinState == LOW) {
        ledPinState = HIGH;
      } else {
        ledPinState = LOW;
      }
      // Apply the new state to the pin
      digitalWrite(ledPin, ledPinState);
    }
  }
  // --- END BLINKING LOGIC ---
  
  delay(1); 
}