/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 2

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

Adafruit_NeoPixel pixels(NUMPIXELS, D8, NEO_RGB + NEO_KHZ800);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pixels.begin();

  // prepare LED

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  bool connoctAni = false;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(F("."));
    
    pixels.setPixelColor(0, connoctAni ? 0xff0000 : 0);
    pixels.setPixelColor(1, connoctAni ? 0 : 0xff0000);
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

  if (req.indexOf(F("favicon")) != -1) { return; }
  Serial.println(req);

  if (req.indexOf(F("GET / ")) == 0) {
    client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));
    client.print(F(
"<!doctype html>\n"
"<html>\n"
"<body>\n"
"<input type='color' name='color0'/>\n"
"<input type='color' name='color1'/>\n"
"<style>input{width: 100%;height: 30vh;}</style>\n"
"<script>\n"
"  const nodes = [...document.querySelectorAll('input')];\n"
"  const query = (name, value='') => {\n"
"    fetch(`/${name}/${value.substr(1)}`)\n"
"      .then(res=>res.json())\n"
"      .then(data => {\n"
"        data.colors.forEach((color, index) => {\n"
"          nodes[index].value = '#' + ('000000' + color.toString(16)).substr(-6);\n"
"        });\n"
"      });\n"
"  };\n"
"  nodes.forEach(node => {\n"
"    node.addEventListener('change', ({target}) => {\n"
"      query(target.name, target.value);\n"
"    });\n"
"  });\n"
"  query('initial');\n"
"</script>\n"
"</body>\n"
"</html>\n"
    ));
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

long storeColor(String query, byte index) {
  int offset = req.indexOf(query);
  if (offset != -1) {
    String colorRaw = req.substring(offset + query.length(), offset + query.length() + 6);
    char cBuf[7] = "000000";
    colorRaw.substring(0, 6).toCharArray(cBuf, 7);
    colors[index] = strtoul(cBuf, NULL, 16);
  }
}
