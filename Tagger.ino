// Infinitag Libs
#include <sensor_dhcp_server.h>
#include <Infinitag_SH1106.h>
#include <Infinitag_GFX.h>
#include <Infinitag_Core.h>

// Vendor Libs
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

// Button Settings
const int fireBtnPin = 2;
int fireBtnState = 0;
const int specialBtnPin = 3;
int specialBtnState = 0;

// Settings
const int muzzleLedPin = 8;
const int displayResetPin = 4;
const int displayDcPin = 5;
const int displayCsPin = 6;
bool alive = true;
unsigned long timeOfDeath = 0;

unsigned int intensity = 64;
// Infinitag Inits
SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);
Infinitag_Core infinitagCore;
sh1106_spi display = create_display(displayResetPin, displayDcPin, displayCsPin);
Framebuffer framebuffer;

// Vendor Inits
IRsend irsend;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, muzzleLedPin, NEO_GRBW + NEO_KHZ800);

// Player
byte playerTeamId = 1;
byte playerId = 1;

void setup() {
  Serial.begin(57600);
  
  Serial.println("booting Server...");
  //SensorServer.initialize();
  
  Serial.println("booting Events...");
  Wire.begin();
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

  displayPlayerInfo();
  updateSensorConfig();
}

void loop() {
  fireBtnState = digitalRead(fireBtnPin);
  specialBtnState = digitalRead(specialBtnPin);

  // Demo Player Setup
  if (specialBtnState == HIGH) {
      playerId++;
      if (playerId > 2) {
        playerId = 1;
        playerTeamId++;
      }
      if (playerTeamId > 2) {
        playerId = 1;
        playerTeamId = 1;
      }
      displayPlayerInfo();
      updateSensorConfig();
      delay(150);
  }

  if (alive) {
    colorWipe(strip.Color(0,intensity,0,0));
    
    if (fireBtnState == HIGH) {
      unsigned long shootValue = infinitagCore.ir_encode(false, 0, playerTeamId, playerId, 1, 100);
      irsend.sendRC5(shootValue, 24);
      colorWipe(strip.Color(0,0,intensity,0));
      delay(100);
      colorWipe(strip.Color(0,intensity,0,0));
      //alive = false;
      //timeOfDeath = millis();
    }
  } else {
    /*unsigned long currentTime = millis();
    unsigned long ti = currentTime - timeOfDeath;
    char number[100];
    String(ti/1000).toCharArray(number, 100);
    framebuffer.displayText(number, 55, 24, WHITE);
    if (ti > 500) {
      colorWipe(strip.Color(0,0,0,0));
    }
    if (ti > 20000) { //20 seconds
      alive = true; 
    }*/
  }
  //SensorServer.scanIfNecessary();
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

void displayPlayerInfo () {
  framebuffer.clear(BLACK);
  String displayPlayerText = "Team ";
  displayPlayerText += playerTeamId;
  displayPlayerText += " / Player ";
  displayPlayerText += playerId;
  char charBuf[50];
  displayPlayerText.toCharArray(charBuf, 50);
  framebuffer.displayText(charBuf, 0, 12, WHITE);
  framebuffer.displayText("Alive", 50, 24, WHITE);
  display_buffer(&display, framebuffer.getData());
}

void updateSensorConfig () {
  Wire.beginTransmission(0x22);
  Wire.write(playerTeamId);
  Wire.write(playerId);
  Wire.endTransmission();
  Wire.beginTransmission(0x24);
  Wire.write(playerTeamId);
  Wire.write(playerId);
  Wire.endTransmission();
}


