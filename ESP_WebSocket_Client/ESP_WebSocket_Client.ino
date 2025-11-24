// ESP WebSocket Client

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <sensorShieldLib.h>
#include <ArduinoJson.h>

#define CLIENT_ID 1
#define PRINT_LOGS false

const char* ssid = "feather32";
const char* password = "feather32";

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
SensorShield board;

int getClientID() {
  return CLIENT_ID;
}

int secondsPassed() {
	return (millis() / 10000) * 10 ;
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      if(PRINT_LOGS) Serial.printf("[LOG] Disconnected!\n");
      break;

    case WStype_CONNECTED:
      if(PRINT_LOGS) Serial.printf("[LOG] Connected to url: %s\n", payload);
      break;

    case WStype_TEXT:
      {
        if(PRINT_LOGS) Serial.printf("[LOG] get text: %s\n", payload);
        JsonDocument doc;
        char raw[length];
        memcpy(raw, payload, length);
        DeserializationError error = deserializeJson(doc, raw);

        if (error) {
          Serial.print(F("[LOG] deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
        else {
          int clientID = doc["clientID"];
          if(clientID != CLIENT_ID) {
            // do something if message from another client
            Serial.printf("%s\n", payload);
          }
        }
      }
      break;

    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("CLIENT_ID ");
  Serial.println(CLIENT_ID);

  WiFiMulti.addAP(ssid, password);

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.4.1", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  board.init(Serial);
  board.addSensor("clientID", getClientID);
  board.addSensor("seconds", secondsPassed);
}

void loop() {
  webSocket.loop();

  board.update(false);
  if(board.hasNewValue == true) {
    webSocket.sendTXT(board.JSONMessage);
  }
}
