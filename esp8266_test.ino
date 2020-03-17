#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <FastLED.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define NUM_LEDS 6
#define DATA_PIN 14

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#ifndef STASSID
#define STASSID "#####"
#define STAPSK  "#####"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
CRGB leds[NUM_LEDS];

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  server.send(200, "text/html", "<html><body><a href=\"on\"><button>on</button></a><br><a href=\"off\"><button>off</button></a></body></html>");
}

void lightOn() {

  for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
  }
  
  FastLED.show();
  handleRoot();
}

void lightOff() {
  for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
  }
  FastLED.show();
  handleRoot();
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  
  display.println("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  



  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  
  display.println("Connected to ");
  display.println(ssid);
  display.println("IP address: ");
  display.println(WiFi.localIP());
  display.display();
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/on", lightOn);
  server.on("/off", lightOff);


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
