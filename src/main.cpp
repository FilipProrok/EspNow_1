#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <esp_now.h>

const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 64;
const uint8_t SCREEN_RESET = -1; // ekran nie ma pinu RESET
const uint8_t SCREEN_ADDRES = 0x3C; // ewentualnie może być 0x3D

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_RESET);

const uint8_t PIN_BUTTON = 14;

typedef struct CommandData{ // moja struktura do wysyłania komendy w celu rządania danych
  uint8_t command;
};

typedef struct SensorData{
  float temperature;
  float humidity;
};

CommandData commandData;
SensorData sensorData;

uint8_t brodcastAddress[] = {0xC0, 0x49, 0xEF, 0xCA, 0x70, 0x24}; // adres drugiego modułu

void setup_esp_now();
void setup_screen();
void OnDataSend(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incoming_data,  int len);

void setup() {
  Serial.begin(115200);
  setup_screen();
  setup_esp_now();
  pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void loop() {
  
}

void setup_esp_now(){
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSend);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo;

  // Register peer
  memcpy(peerInfo.peer_addr, brodcastAddress, 6);
  peerInfo.channel = 0; 
  peerInfo.encrypt = false;
    // Add peer       
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void setup_screen(){
  screen.begin();
  if(!screen.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRES)){
    Serial.println(F("SSD1306 alokacja nieudana lub wyświetlacz nieznaleziony."));
    for(;;);
  }

  screen.setTextSize(1);
  screen.setTextColor(SSD1306_WHITE);
  screen.clearDisplay();
  screen.setCursor(0,0);
}


void OnDataSend(const uint8_t *mac_addr, esp_now_send_status_t status){
  if(status != ESP_OK){
    Serial.println("problem z wysłaniem danych");
    return;
  }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incoming_data,  int len){
  memcpy(&sensorData, incoming_data, sizeof(sensorData));
  setup_screen();
  screen.print("temperature: ");
  screen.println(sensorData.temperature);
  screen.display();
}