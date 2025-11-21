#include <WiFi.h>
#include <WebServer.h>

// --- Configuration Settings ---
// !!! CRITICAL: UPDATE THESE WITH YOUR NETWORK DETAILS !!!
const char* ssid = "<your_SSID>";
const char* password = "<your_PASSWORD>";

// Create a WebServer object on port 80 (standard HTTP port)
WebServer server(80);

// --- Handler Function ---

// Function to handle the root URL ("/") request
void handleRoot() {
  // HTML content for the welcome message
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Welcome</title>";
  html += "<style>";
  html += "body { font-family: sans-serif; text-align: center; margin-top: 100px; background-color: #e6f7ff; }";
  html += "h1 { color: #007bff; font-size: 3em; }";
  html += "p { color: #555; font-size: 1.2em; }";
  html += "</style></head>";
  
  html += "<body>";
  html += "<h1>Welcome to the ESP32 Web Server!</h1>";
  html += "<p>This is a basic server running on port 80.</p>";
  html += "</body></html>";
  
  // Send the HTML response with status code 200 (OK)
  server.send(200, "text/html", html);
}

// Function to handle 404 Not Found errors
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// --- Setup and Loop ---

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  
  // Start the connection process
  WiFi.begin(ssid, password);

  // Wait for connection to succeed (blocking until connected)
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Connection successful!
  Serial.println("\nWiFi connected successfully!");
  Serial.print("Server IP address: ");
  // Print the IP address to the Serial Monitor so you know where to navigate
  Serial.println(WiFi.localIP());

  // Configure Web Server Routes
  server.on("/", handleRoot);       // Map the root path to handleRoot function
  server.onNotFound(handleNotFound); // Map any other path to 404 handler

  // Start the web server
  server.begin();
  Serial.println("HTTP server started on port 80.");
}

void loop() {
  // CRITICAL: Must be called repeatedly to process incoming client requests
  server.handleClient();
  
  // Allows the Wi-Fi and FreeRTOS tasks time to run, improving responsiveness
  delay(1); 
}