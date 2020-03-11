#include "FS.h"
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 2

bool connoctAni = false;

Adafruit_NeoPixel pixels(NUMPIXELS, D8, NEO_RGB + NEO_KHZ800);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pixels.begin();

  StaticJsonDocument<256> conf;
  SPIFFS.begin();
  
  File confFile = SPIFFS.open( "/conf.json", "r"); // Datei zum lesen öffnen
  while (!confFile) {
    Serial.println("opening conf.json failed");
    pixels.setPixelColor(0, connoctAni ? 0xff0000 : 0);
    pixels.setPixelColor(1, connoctAni ? 0 : 0xff0000);
    pixels.show();
    delay(250);
    connoctAni = !connoctAni;
  }

  deserializeJson(conf, confFile.readString());
  confFile.close();

  const char* ssid = conf["STASSID"];
  const char* password = conf["STAPSK"];

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(F("."));
    pixels.setPixelColor(0, connoctAni ? 0x00ff00 : 0);
    pixels.setPixelColor(1, connoctAni ? 0 : 0x00ff00);
    pixels.show();
    connoctAni = !connoctAni;
  }

  pixels.setPixelColor(0, 0);
  pixels.setPixelColor(1, 0);
  pixels.show();

  Serial.println();
  Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());
}

String req;
uint32_t colors[2];

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) { return; }

  client.setTimeout(5000); // default is 1000

  // Read the first line of the request
  req = client.readStringUntil('\r');
  Serial.println(req);

  if ((req.indexOf(F("GET /")) == 0) && (req.indexOf(F("GET /color")) == -1)) {
    //sendFileContent(client, "index.html");
    sendFileContent(client, req.substring(4, req.indexOf(" ", 5)));
    return;
  }

  storeColor(F("/color0/"), 0);
  storeColor(F("/color1/"), 1);

  while (client.available()) { client.read(); }
  
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"colors\":["));
  client.print(colors[0]);
  client.print(F(","));
  client.print(colors[1]);
  client.print(F("]}"));

  for(int i=0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colors[i]);
  }
  pixels.show();
  
}

void sendFileContent(WiFiClient client, String filename) {
  if (filename == "/") {
    filename = F("/index.html");
  }

  File serveFile;

  // do not deliver conf.json
  if (filename != F("/conf.json")) {
    serveFile = SPIFFS.open(filename, "r");  
  }

  if (!serveFile) {
    client.print(F("HTTP/1.1 404 REKT\r\nContent-Type: text/plain\r\n\r\n"));
    client.print(filename);
    client.print(F(" not found"));
    return;
  }
  String extension = filename.substring(filename.indexOf(".") + 1);
  client.print(F("HTTP/1.1 200 OK\r\n"));
  client.print(F("Content-Type: "));
  client.print(getMimeType(extension));
  client.print(F("\r\n"));
  client.print(F("\r\n"));
  client.print(serveFile.readString());
  serveFile.close();
}

String getMimeType(String extension) {
  // ToDo...
  // if (extension == F("ico")) {
  //   return F("image/x-icon");
  // }
  
  if (extension == F("html") || extension == F("htm")) {
    return F("text/html");
  }
  
  if (extension == F("css")) {
    return F("text/css");
  }
  
  if (extension == F("js")) {
    return F("text/javascript");
  }
  
  return F("application/octet-stream");
}

long storeColor(String query, byte index) {
  int offset = req.indexOf(query);
  if (offset != -1) {
    String colorRaw = req.substring(offset + query.length(), offset + query.length() + 6);
    char cBuf[7] = "000000";
    colorRaw.substring(0, 6).toCharArray(cBuf, 7);
    colors[index] = strtoul(cBuf, NULL, 16);
  }
}
