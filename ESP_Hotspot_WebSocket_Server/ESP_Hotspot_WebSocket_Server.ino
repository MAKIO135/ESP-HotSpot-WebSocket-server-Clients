// Creates a Wifi Hotspot and init a WebSocket Server that broadcasts incoming messages

// https://www.upesy.fr/blogs/tutorials/how-create-a-wifi-acces-point-with-esp32
// https://shawnhymel.com/1675/arduino-websocket-server-using-an-esp32/

#include <WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "feather32";
const char* password = "feather32";

WebSocketsServer webSocket = WebSocketsServer(80);

void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t* payload,
                      size_t length) {

  // Figure out the type of WebSocket event
  switch (type) {
    Serial.println(type);

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;

    // broadcast text message to all clients
    case WStype_TEXT:
      Serial.printf("[%u] Text: %s\n", num, payload);
      webSocket.broadcastTXT(payload);
      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());

  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  // Look for and handle WebSocket data
  webSocket.loop();
}
