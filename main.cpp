#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#include "canbus.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Ticker tempTimer;
Ticker pinState;
Ticker wsCleanupTimer;

const char *ssid = "abc";
const char *password = "abc";

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void notFound(AsyncWebServerRequest *request);
void ws_tempESP32(String temp);
void ESP32_tempFlag(void);
void ws_cleanupFlag(void);
void setPin(void);

// Flags
uint8_t tempAvailable = 0;
uint8_t wsCleanupAvailable = 0;

void setup()
{
  Serial.begin(115200);
  uint32_t psram = ESP.getPsramSize();
  Serial.println(psram);

  // Led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinState.attach_ms(500, setPin);

  // SPI_FFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi Failed!");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // mDNS
  if (MDNS.begin("myserver"))
  {
    Serial.println("mDNS responder started");
  }
  else
  {
    Serial.println("Error in mDNS");
  }

  // Temperature
  tempTimer.attach_ms(1000, ESP32_tempFlag);

  // Webserver ---------------------------------------------------------------
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello, world");
  });

  server.onNotFound(notFound);

  // server.serveStatic("/index.html", SPIFFS, "/index.html").setCacheControl("max-age=5");
  server.serveStatic("/index.html", SPIFFS, "/index.html");
  server.serveStatic("/js_code.js", SPIFFS, "/js_code.js");

  server.begin();

  wsCleanupTimer.attach_ms(1000, ws_cleanupFlag);
  CAN_begin();
}

void loop()
{
  // Read internal temperature ESP32
  if (tempAvailable)
  {
    String tempESP32 = String(temperatureRead(), 2);
    ws_tempESP32(tempESP32);
    tempAvailable = 0;
  }

  CAN_read();

  // Clean old websocket clients
  if (wsCleanupAvailable)
  {
    ws.cleanupClients();
    wsCleanupAvailable = 0;
  }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void ws_tempESP32(String temp)
{
  DynamicJsonDocument msgServer(50);
  char tempJson[50];

  msgServer["temp"] = temp;
  serializeJson(msgServer, tempJson);
  ws.textAll(tempJson);
  Serial.println(tempJson);
}

void ws_cleanupFlag(void)
{
  wsCleanupAvailable = 1;
}

void ESP32_tempFlag(void)
{
  tempAvailable = 1;
}

void setPin(void)
{
  digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  //Handle WebSocket event
  client->text("Hello from server!");
}