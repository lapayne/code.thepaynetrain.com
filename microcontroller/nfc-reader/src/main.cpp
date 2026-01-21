#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>


#define RST_PIN         3           
#define SS_PIN          7           

MFRC522 mfrc522(SS_PIN, RST_PIN);

//RGB LED Pins
#define LED_PIN_R 20 // Red
#define LED_PIN_G 0  // Green
#define LED_PIN_B 1  // Blue

// --- AUTHORIZATION SETTINGS ---
// Add your authorized UIDs here (Must be UPPERCASE)
String allowedUIDs[] = {"0496DA753E6180", "5DA85A06"}; 
int allowedCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct {
    char uid[32];   // adjust size to match your UID
} esp_now_data_t;

esp_now_data_t outgoingData;


void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
    Serial.print("ESP-NOW Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}


esp_now_peer_info_t peerInfo;

void flashRed() {
  analogWrite(LED_PIN_B, 0);
  for (int i = 0; i < 4; i++) {
    analogWrite(LED_PIN_R, 255);
    delay(150);
    analogWrite(LED_PIN_R, 0);
    delay(150);
     
  }
  analogWrite(LED_PIN_B, 255);
}

// Helper function to convert Hue to RGB and write to pins
void setHue(int hue) { // hue: 0 to 360
  int r, g, b;
  float s = 1.0, v = 1.0; // Max saturation and brightness

  float c = v * s;
  float x = c * (1 - abs(fmod(hue / 60.0, 2) - 1));
  float m = v - c;
  float rf, gf, bf;

  if (hue < 60) { rf = c; gf = x; bf = 0; }
  else if (hue < 120) { rf = x; gf = c; bf = 0; }
  else if (hue < 180) { rf = 0; gf = c; bf = x; }
  else if (hue < 240) { rf = 0; gf = x; bf = c; }
  else if (hue < 300) { rf = x; gf = 0; bf = c; }
  else { rf = c; gf = 0; bf = x; }

  // Convert to 0-255 for analogWrite
  analogWrite(LED_PIN_R, (rf + m) * 255);
  analogWrite(LED_PIN_G, (gf + m) * 255);
  analogWrite(LED_PIN_B, (bf + m) * 255);
}

void rainbowCycle(int ms) {
  for (int h = 0; h < 360; h++) {
    setHue(h);
    delay(ms / 360); // Distribute duration across 360 degrees
  }
  // Turn off after cycle
  analogWrite(LED_PIN_R, 0);
  analogWrite(LED_PIN_G, 0);
  analogWrite(LED_PIN_B, 0);
}

void setPurple() {
  analogWrite(LED_PIN_R, 255);
  analogWrite(LED_PIN_G, 0);
  analogWrite(LED_PIN_B, 255);
  delay(2000);
}


void setup() {
  Serial.begin(115200);
  while (!Serial);

  SPI.begin(4, 5, 6, 7);
  mfrc522.PCD_Init();
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_max_tx_power(40);

  if (esp_now_init() != ESP_OK) return;

  esp_now_register_send_cb(OnDataSent);
  
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  pinMode(LED_PIN_B, OUTPUT);
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);




}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // 1. Get UID from tag
  String tempUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    tempUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    tempUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  tempUID.toUpperCase();

  // 2. Check if UID is in the allowed array
  bool authorized = false;
  for (int i = 0; i < allowedCount; i++) {
    if (tempUID == allowedUIDs[i]) {
      authorized = true;
      break;
    }
  }

  
  memset(outgoingData.uid, 0, sizeof(outgoingData.uid));
  tempUID.toCharArray(outgoingData.uid, sizeof(outgoingData.uid));

  esp_err_t result = esp_now_send(
    broadcastAddress,
    (uint8_t*)&outgoingData,
    sizeof(outgoingData)
  );

  if (result != ESP_OK) {
    Serial.println("ESP-NOW send failed");
  }

  // 3. Logic based on authorization
if (authorized) {
  Serial.println("Access Granted: " + tempUID);


  analogWrite(LED_PIN_B, 0);
  analogWrite(LED_PIN_G, 255);
  delay(2000);
  analogWrite(LED_PIN_G, 0);
  delay(150);
  rainbowCycle(5000);
  analogWrite(LED_PIN_B, 255);
}
  else {
    Serial.println("Access Denied: " + tempUID);
    flashRed(); 
  }

  

  mfrc522.PICC_HaltA();
  delay(1000); 
}