#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

typedef struct struct_message {
  char uid[32]; 
} struct_message;

struct_message incomingData;

// This signature is ONLY valid in Arduino Core v3.0.0+
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  
  Serial.println("\n--- MESSAGE RECEIVED (v3) ---");
  Serial.printf("Tag UID: %s\n", incomingData.uid);
  
  // Access MAC from the info struct
  Serial.print("From: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X%s", recv_info->src_addr[i], (i < 5) ? ":" : "");
  }
  
  // Access RSSI (Signal Strength)
Serial.printf("\nRSSI: %d dBm\n", recv_info->rx_ctrl->rssi);
  Serial.println("---------------------------");
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA); 
  
  // Set to Channel 1
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  // Register the callback
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver Ready (v3 Core) on Channel 1");
}

void loop() {
  yield(); 
}