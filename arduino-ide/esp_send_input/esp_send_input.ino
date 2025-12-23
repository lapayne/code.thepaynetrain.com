#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Keypad.h>

// 1. Define the Keymap
const byte ROWS = 4; // four rows
const byte COLS = 3; // three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// 2. Assign the ESP32-C3 Pins (Avoiding Strapping Pins 2, 8, 9)
byte rowPins[ROWS] = {3, 4, 1, 2}; 
byte colPins[COLS] = {10, 20, 21};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  char uid[32];
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// CORRECTED FOR CORE 3.3.5
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  
  // Wait for Serial on C3
  unsigned long startWait = millis();
  while (!Serial && millis() - startWait < 5000); 

  Serial.println("ESP32-C3 Sender Starting...");

  WiFi.mode(WIFI_STA);
   WiFi.setTxPower(WIFI_POWER_8_5dBm);
  // 2. Force Channel 1
  // This is the critical part to fix the "Delivery Fail" on the sender side
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback - no casting needed now
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memset(&peerInfo, 0, sizeof(peerInfo)); // Best practice: clear the struct first
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {

  char key = keypad.getKey();

  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
  }
  //strcpy(myData.uid, "A2 B4 C8 D9 A0");

  //esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  
}