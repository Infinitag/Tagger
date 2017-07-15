// Infinitag Libs
#include <sensor_dhcp_server.h>
#include <Infinitag_SH1106.h>
#include <Infinitag_GFX.h>
#include <Infinitag_Core.h>

// Vendor Libs
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

// Button Settings
const int rightBtnPin = 34;
int rightBtnState = 0;
const int leftBtnPin = 35;
int leftBtnState = 0;
const int downBtnPin = 36;
int downBtnState = 0;
const int upBtnPin = 37;
int upBtnState = 0;
const int specialBtnPin = 44;
int specialBtnState = 0;
const int infoBtnPin = 45;
int infoBtnState = 0;
const int reloadBtnPin = 46;
int reloadBtnState = 0;
const int fireBtnPin = 47;
int fireBtnState = 0;
const int enterBtnPin = 48;
int enterBtnState = 0;
const int resetBtnPin = 49;
int resetBtnState = 0;

// Settings
const int muzzleLedPin = 8;
const int displayResetPin = 4;
const int displayDcPin = 5;
const int displayCsPin = 6;
bool alive = true;
unsigned long timeOfDeath = 0;

unsigned int intensity = 255;
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
  Serial.println(fireBtnState);
  specialBtnState = digitalRead(specialBtnPin);
  colorWipe(strip.Color(0,0,intensity,0));

  if (alive) {
    
    if (fireBtnState == HIGH) {
      unsigned long shootValue = infinitagCore.ir_encode(false, 0, playerTeamId, playerId, 1, 100);
      irsend.sendRC5(shootValue, 24);
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
  delay(100);
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

void sendCmd(byte data[], unsigned int byteLength) {
  // Ziel muss noch bestimmt werden
  Wire.beginTransmission(0x22);
  Wire.write(data, byteLength);
  Wire.endTransmission();
}

void sendCmdSetGameId(unsigned int gameId) {
  byte data[2] = {
    0x01, 
    gameId
  };
  sendCmd(data, 2);
}

void sendCmdSetPlayerId(unsigned int playerId) {
  byte data[2] = {
    0x02, 
    playerId
  };
  sendCmd(data, 2);
}

void sendCmdSetSensorId(unsigned int sensorId) {
  byte data[2] = {
    0x03, 
    sensorId
  };
  sendCmd(data, 2);
}

void sendCmdSetAnimation(unsigned int animationId, unsigned int duration, unsigned int colorR, unsigned int colorG, unsigned int colorB, unsigned int colorW, unsigned int repeat) {
  byte data[9] = {
    0x04, 
    animationId, 
    duration, 
    duration >> 8, 
    colorR, 
    colorG, 
    colorB, 
    colorW, 
    repeat
  };
  sendCmd(data, 9);
}


void sendCmdPing(unsigned int senderId) {
  byte data[2] = {
    0x05, 
    senderId
  };
  sendCmd(data, 2);
}

