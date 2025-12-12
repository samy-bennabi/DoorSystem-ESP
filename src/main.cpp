#include <SPI.h>
#include <MFRC522.h>

#include "myWiFi.h"
#include "myHttp.h"
#include "myMqtt.h"
#include "myRfid.h"
#define RELAY_PIN 15 // pin for relay
#define RST_PIN 39 // Reset pin
#define SS_PIN 5  // Slave select pin

#define LED_GREEN_PIN 2
#define LED_YELLOW_PIN 4
#define LED_RED_PIN 17

const char *ssid = "ssid";
const char *password = "password";


const char *doorName        = "C-089";
const int   door_open_delay = 2000;

const char *apiServer       = "api.servername.com";
const int   apiPort         = 8080;
const char *api_card_add    = "/api/card/add";
const char *api_access_add  = "/api/access/add";
const char *api_access_check= "/api/access/check";

const char *mqttServer      = "mqtt.servername.com";
const int   mqttPort        = 1883;
const char *mqtt_id         = "ESP32";
const char *mqtt_user       = "user";
const char *mqtt_password   = "Patate123";
const char *mqtt_door_open  = "DoorSystem/door/open";

MFRC522 mfrc522(SS_PIN, RST_PIN);

String rfidCard = "";

String readRfidCard();

//MyRfid rfid(SS_PIN, RST_PIN);
MyWiFi wifi;
MyHttp http(apiServer, apiPort);
MyMqtt mqtt(mqttServer, mqttPort, mqtt_id, "", "", doorName);

void setup(){
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  digitalWrite(LED_YELLOW_PIN, LOW);
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, HIGH);
  
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);

  if( !wifi.connectToWiFi(ssid, password)){
    wifi.startAPMode("ESP32", "Patate123");
  }
  delay(100);

  mqtt.mqttSubOpen = mqtt_door_open;
  mqtt.mqttSubAddCard = mqtt_card_add;
   
  mqtt.mqttSubAddAccess = mqtt_access_add;
  mqtt.setup();

  while (!Serial)
    ; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  Serial.println("Lessgo scan...");
}


void loop(){
  mqtt.refresh();

  if (mfrc522.PICC_IsNewCardPresent()) {
    rfidCard = readRfidCard();
    Serial.print("UID tag: ");
    Serial.println(rfidCard);
    
    if(mqtt.addCard==0 & mqtt.addAccess==0){ http.sendPostReq(api_access_check, "cardUid", rfidCard.c_str(), "doorName", doorName); }
    if(mqtt.addCard > 0){ http.sendPostReq(api_card_add, "uid", rfidCard.c_str(), "doorName", doorName); }
    if(mqtt.addAccess > 0){ http.sendPostReq(api_access_add, "cardUid", rfidCard.c_str(), "doorName", doorName); }
  }

  mqtt.loop();
  
  if (mqtt.open) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, LOW);
    delay(2000);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);
    mqtt.open=0;
  }

  
  if(mqtt.addCard > 0){
    mqtt.addCard --;
    digitalWrite(LED_YELLOW_PIN, HIGH);
    Serial.print(mqtt.addCard);
    Serial.println(" seconds remaining, Next card will be added .... ");
    delay (250);
    digitalWrite(LED_YELLOW_PIN, LOW);
  }

  if(mqtt.addAccess > 0){
    mqtt.addAccess --;
    digitalWrite(LED_YELLOW_PIN, HIGH);
    Serial.print(mqtt.addAccess);
    Serial.println(" seconds remaining, Next card will be authorised .... ");
    delay (250);
    digitalWrite(LED_YELLOW_PIN, LOW);
  }

  delay(250);
}

String readRfidCard() {
  String card = "";
  mfrc522.PICC_ReadCardSerial();
  for (byte i = 0; i < mfrc522.uid.size; i++){
    card.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    card.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  return card;
}