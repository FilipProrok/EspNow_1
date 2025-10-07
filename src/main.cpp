#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

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

uint8_t brodcastAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // adres drugiego modułu

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
  if(digitalRead(PIN_BUTTON) == LOW){
    Serial.println("Przycisk przyciśnięty - próba wysłania");
    
    commandData.command = 1;
    esp_err_t result = esp_now_send(brodcastAddress, (uint8_t*) &commandData, sizeof(commandData));

    if (result != ESP_OK) {
      Serial.print("Błąd esp_now_send: ");
      // Możesz dodać więcej szczegółów w zależności od zwracanego błędu (np. ESP_ERR_ESPNOW_IF).
      // W tym prostym przypadku wystarczy to:
      Serial.println(result); 
    } else {
       // Nie ma sensu wypisywać tego tutaj, bo OnDataSend załatwia sprawę
    }
    
    delay(50); // Zabezpieczenie przed zalaniem
  }
}

void setup_esp_now(){
  WiFi.mode(WIFI_STA);

  //esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSend);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo;

  memset(&peerInfo, 0, sizeof(peerInfo)); 

  // Register peer
  memcpy(peerInfo.peer_addr, brodcastAddress, 6);
  peerInfo.channel = 0; 
  peerInfo.encrypt = false;
    // Add peer       
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Nie udało sie dodać peera");
    return;
  }
  Serial.println("Peer powienien sie dodać kurde");
}

void setup_screen(){
  if(!screen.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRES)){
    Serial.println(F("SSD1306 alokacja nieudana lub wyświetlacz nieznaleziony."));
    for(;;);
  }

  screen.setTextSize(1);
  screen.setTextColor(SSD1306_WHITE);
  screen.clearDisplay();
  screen.setCursor(0,0);

  screen.println("Chyba dziala");
  screen.display(); // Wymagane, aby wyświetlić tekst!
  delay(3000);
  screen.clearDisplay();
  screen.display(); // Wymagane, aby wyczyścić ekran!
}


void OnDataSend(const uint8_t *mac_addr, esp_now_send_status_t status){
  // Deklaracja MAC-a jako string do czytania
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  if(status == ESP_OK){
    Serial.print("Pakiet do MAC: ");
    Serial.print(macStr);
    Serial.println(" wysłany pomyślnie.");
  } else {
    Serial.print("Błąd wysłania pakietu do MAC: ");
    Serial.println(macStr);
  }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incoming_data,  int len){
  memcpy(&sensorData, incoming_data, sizeof(sensorData));
  Serial.println("Dane przyszły");
  // Niezbędne kroki, aby dane były czytelne
  screen.clearDisplay(); // Wyczyść bufor
  screen.setCursor(0, 0); // Ustaw kursor na początek
  
  screen.print("temperature: ");
  screen.println(sensorData.temperature);
  screen.display(); // Wyślij bufor na ekran
}
