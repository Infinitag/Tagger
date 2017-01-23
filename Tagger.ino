// Infinitag Libs
#include <sensor_dhcp_server.h>
#include <Infinitag_SH1106.h>
#include <Infinitag_GFX.h>
#include <Infinitag_Core.h>
#include <IRremote.h>

// Vendor Libs
#include <Adafruit_NeoPixel.h>

// Settings
const int fireBtnPin = 2;
int fireBtnState = 0;
const int muzzleLedPin = 8;
const int displayResetPin = 4;
const int displayDcPin = 5;
const int displayCsPin = 6;
bool alive = true;
unsigned long timeOfDeath = 0;

// Infinitag Inits
SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);
Infinitag_Core infinitagCore;
sh1106_spi display = create_display(displayResetPin, displayDcPin, displayCsPin);
Framebuffer framebuffer;

// Vendor Inits
IRsend irsend;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, muzzleLedPin, NEO_GRBW + NEO_KHZ800);

void setup() {
  Serial.begin(57600);
  
  Serial.println("booting Server...");
  SensorServer.initialize();
  
  Serial.println("booting Events...");
  Wire.onRequest(requestEvent);

  Serial.println("booting Pins...");
  pinMode(fireBtnPin, INPUT);
  pinMode(muzzleLedPin, OUTPUT);

  Serial.println("booting Display...");
  SPI.begin();
  initialize_display(&display);
  
  Serial.println("booting LEDs...");
  strip.begin();
  colorWipe(strip.Color(0,0,0,0));
  
  Serial.println("boot completed");
}

void loop() {
  framebuffer.clear(BLACK);
  fireBtnState = digitalRead(fireBtnPin);

  if (alive) {
    framebuffer.displayText("Alive", 50, 24, WHITE);
    colorWipe(strip.Color(0,255,0,0));
    
    if (fireBtnState == HIGH) {
      
      unsigned long shootValue = infinitagCore.ir_encode(false, 0, 2, 4, 1, 100);
      irsend.sendRC5(shootValue, 24);
      colorWipe(strip.Color(0,0,255,0));
      delay(100);
      colorWipe(strip.Color(0,255,0,0));
      //alive = false;
      //timeOfDeath = millis();
    }
  } else {
    unsigned long currentTime = millis();
    unsigned long ti = currentTime - timeOfDeath;
    char number[100];
    String(ti/1000).toCharArray(number, 100);
    framebuffer.displayText(number, 55, 24, WHITE);
    if (ti > 500) {
      colorWipe(strip.Color(0,0,0,0));
    }
    if (ti > 20000) { //20 seconds
      alive = true; 
    }
  }
  SensorServer.scanIfNecessary();
  display_buffer(&display, framebuffer.getData());
  delay(10);
}

void requestEvent() {
  //maybe we have enough time to look for the next free address?
  Serial.println("requested");
  SensorServer.registerNewClient();
}

void colorWipe(uint32_t c) {
  for (uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}
