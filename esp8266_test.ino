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
#define BRIGHTNESS 90
#define FRAMES_PER_SECOND  120


#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#ifndef STASSID
#define STASSID "####"
#define STAPSK  "####"
#endif

uint8_t ledMode = 0;

const char* ssid = STASSID;
const char* password = STAPSK;
CRGB leds[NUM_LEDS];
uint8_t gHue = 0;

ESP8266WebServer server(80);

void prepDisplay() {
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
}

void handleRoot() {
  server.send(200, "text/html", "<html><head><style>button{width:200px;height:50px;}</style></head><body><a href=\"light?on\"><button>on</button></a><br><a href=\"light?off\"><button>off</button></a></body></html>");
}

void lightHandler() {  
  if( server.hasArg("on") ) {    
    ledMode = 1;
  } else if ( server.hasArg("off") ){
    ledMode = 0;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }
    
    FastLED.show();
  }
  handleRoot();
}

void handleNotFound() {
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
}

void drawTwinkles() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void setup(void) {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    for(;;); // Don't proceed, loop forever
  }

  prepDisplay();
  display.println("Connecting...");
  display.display();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  prepDisplay();  
  display.println(ssid);
  display.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
    display.println("MDNS responder ok");
  }

  server.on("/", handleRoot);

  server.on("/light", HTTP_GET, lightHandler);


  server.onNotFound(handleNotFound);

  server.begin();
  
  display.println("HTTP server started");
  display.display();
}

void loop(void) {
  server.handleClient();
  MDNS.update();

  if(ledMode == 1) {
    drawTwinkles();
    FastLED.show();  
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND); 
  
    EVERY_N_MILLISECONDS( 20 ) { gHue++; }

  } 
}
