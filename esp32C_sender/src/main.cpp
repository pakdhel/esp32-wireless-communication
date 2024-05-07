#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int NUM_SWITCHES = 8;
int DIP_SWITCH_PINS[] = {13,12,14,27,26,25,33,32}; // Definisikan pin GPIO untuk masing-masing dip switch
uint8_t broadcastAddress[] = {0xE8, 0x6B, 0xEA, 0xD4, 0xD4, 0xA0};

LiquidCrystal_I2C lcd(0x27, 16, 2);

int readDipSwitch();
int previousValue = 0;
int dipSwitchValue = 0;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 

  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Inisialisasi pin dip switch sebagai input
  for (int i = 0; i < NUM_SWITCHES; i++) {
    pinMode(DIP_SWITCH_PINS[i], INPUT);
  }
}

void loop() {
  // Baca nilai dari dip switch dan konversi ke nilai desimal
  dipSwitchValue = readDipSwitch();

  // Tampilkan nilai desimal pada Serial Monitor
  Serial.print("esp32 C: ");
  Serial.println(dipSwitchValue);

  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("esp32 C :"); 
  lcd.setCursor(0, 1); 
  lcd.print(dipSwitchValue);

  if (dipSwitchValue != previousValue) { 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("esp32 C :"); 
    lcd.setCursor(0, 1); 
    lcd.print(dipSwitchValue);
    previousValue = dipSwitchValue;
  }

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &dipSwitchValue, sizeof(dipSwitchValue));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }  else {
    Serial.println("Error sending the data");
  }

  delay(1000); // Delay untuk mencegah pembacaan berulang terlalu cepat
}

int readDipSwitch() {
  int dipSwitchValue = 0;

  // Baca nilai dari masing-masing dip switch dan tambahkan ke nilai biner
  for (int i = 0; i < NUM_SWITCHES; i++) {
    int switchState = digitalRead(DIP_SWITCH_PINS[i]);
    dipSwitchValue += switchState << (NUM_SWITCHES - 1 - i);
  }

  return dipSwitchValue;
}

