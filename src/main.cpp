#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>

#define RELAY_PIN 15 // pin for relay
#define RST_PIN 39 // Reset pin
#define SS_PIN 5  // Slave select pin

#define LED_GREEN_PIN 2
#define LED_YELLOW_PIN 4
#define LED_RED_PIN 17

const char *ssid = "ssid";
const char *password = "password";

const char *doorName        = "Server Room";
const int   doorOpenFor = 2000;

const char *apiServerName   = "api.servername.com";
const int   apiPort         = 8080;
const char *apiPathCardAdd    = "/api/card/add";
const char *apiPathAccessAdd  = "/api/access/add";
const char *apiPathAccessCheck= "/api/access/check";

const char *mqttServer      = "mqtt.servername.com";
const int   mqttPort        = 1883;
const char *mqttId         = "ESP32";
const char *mqttUser       = "user";
const char *mqttPassword   = "Patate123";
const char *mqttDoorOpen  = "DoorSystem/door/open";
const char *mqttCardAdd   = "Doorsystem/card/add";
const char *mqttAccessAdd = "Doorsystem/access/add";

MFRC522 mfrc522(SS_PIN, RST_PIN);
HTTPClient httpClient;
WiFiServer wifiServer;

String rfidCard = "";
String readRfidCard();
bool connectToWiFi(const char* ssid, const char* password);
void startAPMode(const char* ssid, const char* password);
void serveWebPage(WiFiClient wifiClient);
int sendHttpPostReq(const char* uri, const char* key1, const char* value1, const char* key2, const char* value2);

//MyRfid rfid(SS_PIN, RST_PIN);
//MyWiFi wifi;
MyMqtt mqtt(mqttServer, mqttPort, mqttId, "", "", doorName);

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

  if( !connectToWiFi(ssid, password)){
    startAPMode("ESP32", "Patate123");
  }
  delay(100);

  mqtt.mqttSubOpen = mqttDoorOpen;
  mqtt.mqttSubAddCard = mqttCardAdd;
  mqtt.mqttSubAddAccess = mqttAccessAdd;
  mqtt.setup();

  while (!Serial)
    ; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4) | idk if I should be keeping this in, gotta check

  Serial.println("Setup finished successfuly.");
}


void loop(){
  mqtt.refresh();

  if (mfrc522.PICC_IsNewCardPresent()) {
    rfidCard = readRfidCard();
    Serial.print("UID tag: ");
    Serial.println(rfidCard);

    /* this block is incompatible with the current workings, will be useful later on.
    if(sendHttpPostReq(apiPathAccessCheck, "cardUid", rfidCard.c_str(), "doorName", doorName)==200){
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_GREEN_PIN, HIGH);
      digitalWrite(LED_RED_PIN, LOW);
      delay(2000);
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, LOW);
      digitalWrite(LED_RED_PIN, HIGH);
    }
    */

    if(mqtt.addCard==0 & mqtt.addAccess==0){ sendHttpPostReq(apiPathAccessCheck, "cardUid", rfidCard.c_str(), "doorName", doorName); }
    if(mqtt.addCard > 0){ sendHttpPostReq(apiPathCardAdd, "uid", rfidCard.c_str(), "doorName", doorName); }
    if(mqtt.addAccess > 0){ sendHttpPostReq(apiPathAccessAdd, "cardUid", rfidCard.c_str(), "doorName", doorName); }
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

bool connectToWiFi(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);  // The WiFi is in station mode
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  Serial.print(ssid);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) {
    Serial.print(".");
    delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    return true;
  } else {
    Serial.println("Connection failed.");
    return false;
  }
}

void startAPMode(const char* ssid, const char* password) {
  WiFi.mode(WIFI_AP); // The WiFi is in access point mode
  WiFi.softAP(apSSID, apPassword);
  Serial.print("Access point mode started with SSID and password:");
  Serial.println(apSSID + String(" ") + apPassword);
  Serial.print("Access point IP address: ");
  Serial.println(WiFi.softAPIP());

  wifiServer.begin();

  while (1) {
    WiFiClient client = wifiServer.available();
    if (client) { serveWebPage(client); }
  }
}

void serveWebPage(WiFiClient wifiClient) {
  String request = wifiClient.readStringUntil('\r');
  //wifiClient.flush();

  wifiClient.println("HTTP/1.1 200 OK");
  wifiClient.println("Content-Type: text/html");
  wifiClient.println("");
  wifiClient.println("<!DOCTYPE html><html>");
  wifiClient.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>WiFi Configuration</title></head>");
  wifiClient.println("<body><h1>Configure WiFi</h1>");
  wifiClient.println("<form method=\"get\" action=\"/config\">");
  wifiClient.println("<label>SSID:</label>");
  wifiClient.println("<input type=\"text\" name=\"ssid\"><br>");
  wifiClient.println("<label>Password:</label>");
  wifiClient.println("<input type=\"password\" name=\"password\"><br>");
  wifiClient.println("<input type=\"submit\" value=\"Submit\">");
  wifiClient.println("</form>");
  wifiClient.println("</body></html>");

  if (request.indexOf("/config") != -1) {
    int ssidStart = request.indexOf("ssid=") + 5;
    int ssidEnd = request.indexOf("&password=");
    int passwordStart = request.indexOf("password=") + 9;
    int passwordEnd = request.indexOf("HTTP/1.1") - 1;

    String ssid = request.substring(ssidStart, ssidEnd);
    String password = request.substring(passwordStart, passwordEnd);

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    WiFi.disconnect();
    delay(1000);
    connectToWiFi(ssid.c_str(), password.c_str());
  }
}

int sendHttpPostReq(const char* uri, const char* key1, const char* value1, const char* key2, const char* value2) {
  httpClient.begin(apiServerName, apiPort, uri);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");

  char postData[100];
  strcpy(postData, key1);
  strcat(postData, "=");
  strcat(postData, value1);
  strcat(postData, "&");
  strcat(postData, key2);
  strcat(postData, "=");
  strcat(postData, value2);

  int httpResponseCode = httpClient.POST(postData);
  return httpResponseCode;
}