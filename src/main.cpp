#include <SPI.h>
#include <MFRC522.h>

#include "myWiFi.h"
#include "myHttp.h"
#include "myMqtt.h"
#include "myRfid.h"

#include "var.h"

MyRfid rfid(SS_PIN, RST_PIN);
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
  
  rfid.setup();
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

  if (rfid.isNewCardPresent()) {
    rfid.readCardSerial();
    Serial.print("UID tag: ");
    Serial.println(rfid.card);
    
    if(mqtt.addCard==0 & mqtt.addAccess==0){ http.sendPostReq(api_access_check, "cardUid", rfid.card.c_str(), "doorName", doorName); }
    if(mqtt.addCard > 0){ http.sendPostReq(api_card_add, "uid", rfid.card.c_str(), "doorName", doorName); }
    if(mqtt.addAccess > 0){ http.sendPostReq(api_access_add, "cardUid", rfid.card.c_str(), "doorName", doorName); }
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