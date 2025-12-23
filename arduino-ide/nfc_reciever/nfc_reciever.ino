#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

typedef struct struct_message {
  char uid[32]; 
} struct_message;

struct_message incomingData;

// BROADCAST CALLBACK
void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  
  Serial.println("\n[BROADCAST RECEIVED]");
  Serial.print("Tag UID: ");
  Serial.println(incomingData.uid);
  
  // Print Sender MAC to verify which device sent it
  Serial.print("From: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X%s", recv_info->src_addr[i], (i < 5) ? ":" : "");
  }
  Serial.println("\n--------------------");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. WiFi setup
  WiFi.mode(WIFI_STA); 
  
  // 2. Force Channel 1 (Sender must also be on Channel 1)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  // 3. Disable Power Saving (Crucial for not missing packets)
  esp_wifi_set_ps(WIFI_PS_NONE);

  // 4. Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Fail");
    return;
  }

  // 5. Register Callback using Core 3.x syntax
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  Serial.println("Receiver Ready (Broadcast Mode) on Channel 1");
}

void loop() {
  delay(1); // Keeps the watchdog happy
}