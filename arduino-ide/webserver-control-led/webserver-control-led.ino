#include <WiFi.h>
#include <WebServer.h>

// --- Configuration Settings ---
// !!! CRITICAL: UPDATE THESE WITH YOUR NETWORK DETAILS !!!
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Define the GPIO pin for the onboard LED on the ESP32-C3 (usually pin 8)
const int ledPin = 8; // Using const int is preferred over #define in modern C++

// Create a WebServer object on port 80 (standard HTTP port)
WebServer server(80);

// Global state variables
// Initial state set to LOW (ON for Active-Low LEDs like on the C3)
int ledState = LOW; 
bool serverRunning = false; // Tracks if the web server has been successfully started

// --- HTML Content Generation ---

// Function to generate the HTML webpage with status and buttons
String getHtmlPage() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32-C3 LED Control</title>";
  html += "<style>";
  html += "body { font-family: sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }";
  html += ".card { background-color: #ffffff; padding: 30px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); display: inline-block; }";
  html += "h1 { color: #333; }";
  html += ".status { padding: 10px; border-radius: 6px; font-weight: bold; margin: 20px 0; font-size: 1.2em; }";
  // The C3 LED is Active-Low, meaning LOW = ON, HIGH = OFF
  html += ".on { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }";
  html += ".off { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }";
  html += ".btn { background-color: #007bff; color: white; border: none; padding: 10px 20px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 8px; transition: background-color 0.3s; }";
  html += ".btn:hover { background-color: #0056b3; }";
  html += "</style></head>";
  
  html += "<body><div class='card'>";
  html += "<h1>ESP32-C3 LED Server</h1>";
  
  // Display current LED status
  // CRITICAL FIX: Check for LOW state, as LOW turns the LED ON (Active-Low)
  if (ledState == LOW) { 
    html += "<p class='status on'>LED Status: ON</p>";
  } else {
    html += "<p class='status off'>LED Status: OFF</p>";
  }
  
  // Buttons to control the LED
  html += "<p><a href='/LED_ON' class='btn'>Turn LED ON</a></p>";
  html += "<p><a href='/LED_OFF' class='btn'>Turn LED OFF</a></p>";
  
  html += "</div></body></html>";
  return html;
}

// --- Handler Functions ---

void handleRoot() {
  server.send(200, "text/html", getHtmlPage());
}

void handleLedOn() {
  // LOW is ON for the C3 LED
  ledState = LOW; 
  digitalWrite(ledPin, ledState);
  // 303 Redirect to the root page to show the new status
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLedOff() {
  // HIGH is OFF for the C3 LED
  ledState = HIGH; 
  digitalWrite(ledPin, ledState);
  // 303 Redirect to the root page to show the new status
  server.sendHeader("Location", "/");
  server.send(303);
}

// If you try to go anywhere but the home page i.e /admin this will return a 404 error back to the browser.
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}


// --- Reconnection Logic (Reintegrated for Stability) ---

// Function to check and handle Wi-Fi connection status and start/stop server
void handleWiFi() {
  // Check if Wi-Fi is NOT connected
  if (WiFi.status() != WL_CONNECTED) {
    // If the server was running, stop it first
    if (serverRunning) {
      serverRunning = false;
      Serial.println("WiFi lost. Web server stopped. Attempting to reconnect...");
    }
    
    // Attempt reconnection 
    static unsigned long lastAttempt = 0;
    // Only attempt to reconnect every 10 seconds to avoid flooding
    if (millis() - lastAttempt > 10000) { 
        WiFi.reconnect();
        lastAttempt = millis();
    }

  } else {
    // Wi-Fi IS connected
    if (!serverRunning) {
      // If server is not running, start it
      
      // Configure Server Routes (needs to be done before server.begin)
      server.on("/", handleRoot);
      server.on("/LED_ON", handleLedOn);
      server.on("/LED_OFF", handleLedOff);
      server.onNotFound(handleNotFound);

      server.begin();
      serverRunning = true;
      Serial.println("\nWiFi connected successfully!");
      Serial.print("Server IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("HTTP server started on port 80.");
    }
  }
}

// --- Setup and Loop ---

void setup() {
  // Start serial communication for debugging at baud 115200
  Serial.begin(115200);
  //sets the PIN to OUTPUT mode
  pinMode(ledPin, OUTPUT);
  // Initialize the physical pin state
  digitalWrite(ledPin, ledState);
  
  // Start the initial connection attempt.
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
}

void loop() {
  // 1. Manage Wi-Fi connection and server state
  handleWiFi();
  
  // 2. Handle client requests ONLY if the server is running
  if (serverRunning) {
    // CRITICAL: Must be called repeatedly to process incoming client requests
    server.handleClient();
  }
  
  // Allows the Wi-Fi and FreeRTOS tasks time to run, improving responsiveness
  delay(1); 
}