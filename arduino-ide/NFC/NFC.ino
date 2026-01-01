#include <SPI.h>
#include <MFRC522.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define RST_PIN         3          
#define SS_PIN          7          

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

#define LED_PIN_1 1 //blue
#define LED_PIN_2 20 // red
#define LED_PIN_3 9 //green

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  char uid[32];
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  Serial.begin(115200);		// Initialize serial communications
  while (!Serial);		    // Wait for serial port to open

  SPI.begin(4, 5, 6, 7);    // Initialize SPI: SCK, MISO, MOSI, SS
  
  mfrc522.PCD_Init();		    // Init MFRC522
  delay(4);				          // Optional delay for some boards
  
  Serial.println(F("**********************************"));
  Serial.println(F("Scan an RFID tag to see its UID..."));
  mfrc522.PCD_DumpVersionToSerial(); // Show details of the PCD - MFRC522 Card Reader details
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

  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);

    Serial.println("BLUE is ON...");
  digitalWrite(LED_PIN_1, HIGH);
  delay(2000);
   digitalWrite(LED_PIN_1, LOW);

      Serial.println("red is ON...");
  digitalWrite(LED_PIN_2, HIGH);
  
  delay(2000);
   digitalWrite(LED_PIN_2, LOW);


  Serial.println("GREEN is ON...");
  digitalWrite(LED_PIN_3, HIGH);
  
  delay(2000);
   digitalWrite(LED_PIN_3, LOW);

}

void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) return;
  if ( ! mfrc522.PICC_ReadCardSerial()) return;

  // Clear previous UID string
  memset(myData.uid, 0, sizeof(myData.uid));

  // Convert UID bytes to a String and store in the struct
  String tempUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    tempUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    tempUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  tempUID.toUpperCase();
  tempUID.toCharArray(myData.uid, 32);

  Serial.print("Sending UID: ");
  Serial.println(myData.uid);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result != ESP_OK) {
    Serial.println("Error sending the data");
  }

  mfrc522.PICC_HaltA(); // Stop reading
  delay(2000); // Prevent duplicate sends
}