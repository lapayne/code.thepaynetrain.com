#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

typedef struct struct_message {
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

struct_message myData;

// UPDATED FOR CORE 3.3.5: The signature for OnDataRecv
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  // Optional: Print the sender's MAC address
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           info->src_addr[0], info->src_addr[1], info->src_addr[2], 
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  Serial.print("Packet from: ");
  Serial.println(macStr);
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("Bool: ");
  Serial.println(myData.d);
  Serial.println("---");
}

void setup() {
  Serial.begin(115200);

  // Critical for ESP32-C3 USB Serial
  unsigned long startWait = millis();
  while (!Serial && millis() - startWait < 5000); 

  Serial.println("\n--- ESP32-C3 Receiver Starting ---");

  // 1. Set WiFi Mode
   WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  // 2. Force Channel 1
  // This is the critical part to fix the "Delivery Fail" on the sender side
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  // 2. Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // 3. Register Callback
  // Note: In Core 3.x, we pass the function directly
  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Ready and waiting for data...");
}
 
void loop() {
  // Receiver stays idle
}