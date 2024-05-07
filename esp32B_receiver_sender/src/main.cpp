#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

int myData; // variabel untuk menyimpan data dari esp32 C
int prevMyData; // variabel sementara

int NUM_SWITCHES = 8;
int DIP_SWITCHES_PINS[] = {13,12,14,27,26,25,33,32};

// D4:8A:FC:60:8D:B0
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0x60, 0x8D, 0xB0};

int readDipSwitch();
int prevDipSwitch = 0;
int dipSwitchValue = 0;
esp_now_peer_info_t peerInfo;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  if (myData != prevMyData) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("esp32 C:");
    lcd.setCursor(13,0);
    lcd.print(myData);
    prevMyData = myData;
  }

  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("int: ");
  Serial.println(myData);
  Serial.println();

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  lcd.init();
  lcd.backlight();
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Error initializing ESP-NOW");
    return;
  }
  
  
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    pinMode(DIP_SWITCHES_PINS[i], INPUT);
  }
}
 
void loop() {
  Serial.print("esp32 B:");
  Serial.println(dipSwitchValue);
  
  lcd.setCursor(0, 1);
  lcd.print("esp32 B:");
  lcd.setCursor(13, 1);
  lcd.print(dipSwitchValue);

  dipSwitchValue = readDipSwitch();
  if (dipSwitchValue != prevDipSwitch) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("esp32 C:");
    lcd.setCursor(13,0);
    lcd.print(myData);
    lcd.setCursor(0, 1);
    lcd.print("esp32 B:");
    lcd.setCursor(13, 1);
    lcd.print(dipSwitchValue);

    Serial.print("esp32 B:");
    Serial.println(dipSwitchValue);
    prevDipSwitch = dipSwitchValue;
    
  }

  myData += dipSwitchValue;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(dipSwitchValue));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }  else {
    Serial.println("Error sending the data");
  }

  delay(1000);
}

int readDipSwitch() {
  int dipSwitchValue = 0;
  for (int i = 0; i < NUM_SWITCHES; i++) {
    int switchState = digitalRead(DIP_SWITCHES_PINS[i]);
    dipSwitchValue += switchState << (NUM_SWITCHES - 1 - i);
  }

  return dipSwitchValue;
}