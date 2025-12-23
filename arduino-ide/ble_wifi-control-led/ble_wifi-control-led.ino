#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> // For standard BLE descriptors

// --- I. Configuration Settings ---

// !!! CRITICAL: UPDATE THESE WITH YOUR NETWORK DETAILS !!!
const char* ssid = "Home";
const char* password = "Tinker11!!";

// BLE (GATT Profile)
// Service UUID for LED Control
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// Characteristic UUID for the LED state (0: OFF, 1: ON)
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
const char* bleDeviceName = "ESP32C3_LED_BLE";

// Hardware
const int ledPin = 8; 

// Create a WebServer object on port 80
WebServer server(80);

// Global state variables
// Initial state: LOW (ON for Active-Low LEDs like on the C3)
int ledState = LOW; 
// BLE Characteristic Object
BLECharacteristic *pCharacteristic;


// --- II. State Management & Helper Functions ---

// Called whenever ledState changes, updates the physical pin and the BLE characteristic
void updateDeviceState(int newState) {
  ledState = newState;
  digitalWrite(ledPin, ledState);
  
  // Update BLE characteristic
  if (pCharacteristic) {
    // Convert ledState (LOW/HIGH) to a command byte (0 or 1) for BLE app readability
    char commandChar = (ledState == LOW) ? '1' : '0'; // LOW='1' (ON), HIGH='0' (OFF)
    pCharacteristic->setValue((uint8_t*)&commandChar, 1);
    pCharacteristic->notify(); // Notify connected clients of the change
  }
}

// Function to generate the HTML webpage with status and buttons (remains the same)
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32-C3 LED Control</title>";
  html += "<style>body { font-family: sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }";
  html += ".card { background-color: #ffffff; padding: 30px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); display: inline-block; }";
  html += "h1 { color: #333; }";
  html += ".status { padding: 10px; border-radius: 6px; font-weight: bold; margin: 20px 0; font-size: 1.2em; }";
  html += ".on { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }";
  html += ".off { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }";
  html += ".btn { background-color: #007bff; color: white; border: none; padding: 10px 20px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 8px; transition: background-color 0.3s; }";
  html += ".btn:hover { background-color: #0056b3; }</style></head>";
  
  html += "<body><div class='card'>";
  html += "<h1>ESP32-C3 LED Server</h1>";
  
  // Display current LED status (LOW = ON)
  if (ledState == LOW) { 
    html += "<p class='status on'>LED Status: ON (Controlled via Wi-Fi or BLE)</p>";
  } else {
    html += "<p class='status off'>LED Status: OFF (Controlled via Wi-Fi or BLE)</p>";
  }
  
  // Buttons to control the LED
  html += "<p><a href='/LED_ON' class='btn'>Turn LED ON (Wi-Fi)</a></p>";
  html += "<p><a href='/LED_OFF' class='btn'>Turn LED OFF (Wi-Fi)</a></p>";
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// --- III. WebServer Handler Functions ---

void handleLedOn() {
  // LOW is ON for the C3 LED
  updateDeviceState(LOW); 
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLedOff() {
  // HIGH is OFF for the C3 LED
  updateDeviceState(HIGH); 
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}


// --- IV. BLE Callback Class ---

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // Get the first byte of the data received
    const uint8_t* data = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();

    if (length > 0) {
      char commandChar = (char)data[0];

      if (commandChar == '1') { // Command '1' means ON
        Serial.println("Received BLE command: ON");
        updateDeviceState(LOW); // LOW is ON
      } else if (commandChar == '0') { // Command '0' means OFF
        Serial.println("Received BLE command: OFF");
        updateDeviceState(HIGH); // HIGH is OFF
      } else {
        Serial.print("Received unknown BLE command: ");
        Serial.println(commandChar);
      }
    }
  }
};


// --- V. Setup Functions ---

void setupBLE() {
  Serial.println("Starting BLE server...");
  BLEDevice::init(bleDeviceName);

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY // Allows app to subscribe to state changes
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  
  // Set initial value
  char initialChar = (ledState == LOW) ? '1' : '0';
 pCharacteristic->setValue((uint8_t*)&initialChar, 1);
  
  // Set the callback to handle writes from the phone
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // Compatibility fixes
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("BLE Advertising started. Scan for 'ESP32C3_LED_BLE'");
}

void setupWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected successfully!");
  Serial.print("Server IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/LED_ON", handleLedOn); 
  server.on("/LED_OFF", handleLedOff); 
  server.onNotFound(handleNotFound); 

  server.begin();
  Serial.println("HTTP server started on port 80.");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  setupWiFi(); // Initialize Wi-Fi and WebServer
  setupBLE();  // Initialize Bluetooth Low Energy
}

// --- VI. Main Loop ---

void loop() {
  // CRITICAL: Must be called repeatedly to process incoming client requests
  server.handleClient();
  
  // Allows the Wi-Fi and FreeRTOS tasks time to run
  delay(1); 
}