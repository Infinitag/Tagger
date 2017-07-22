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
unsigned int currentScreen = 0; // 0 = home / 1 = inGame / 2 = gameStats

// Timings
unsigned long gameTime = 15000;
unsigned long gameTimeStart;
unsigned long gameTimeEnd;
unsigned long gameTimeDiff;
int gameTimeDiffMinutes;
int gameTimeDiffSeconds;
unsigned long gameTimeToEnd;

// Stats
unsigned int statsShots;
unsigned int statsDeath;

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
  SensorServer.initialize();
  
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

  updateSensorConfig();
}

void loop() {
  getButtonStates();

  switch(currentScreen) {
    case 2:
      loopGameStats();
      break;
    case 1:
      loopInGame();
      break;
    case 0:
    default:
      loopHomescreen();
      break;
  }
  
  //SensorServer.scanIfNecessary();
}

void loopHomescreen() {
  framebuffer.clear(BLACK);
  
  String text = "Homescreen";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 0, WHITE);
  framebuffer.drawHorizontalLine(0, 14, 128, WHITE);
  
  text = "Press [Enter] to play";
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 30, WHITE);

  display_buffer(&display, framebuffer.getData());
  
  if (enterBtnState == HIGH) {
    startGame();
    return;
  }
  
  delay(100);
}

void loopGameStats() {
  framebuffer.clear(BLACK);
  
  String text = "Game-Stats";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 0, WHITE);
  framebuffer.drawHorizontalLine(0, 14, 128, WHITE);

  text = "Shots: ";
  text += statsShots;
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 17, WHITE);

  text = "Death: ";
  text += statsDeath;
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 31, WHITE);
  
  text = "Press [Enter] to restart";
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 49, WHITE);
  framebuffer.drawHorizontalLine(0, 48, 128, WHITE);

  display_buffer(&display, framebuffer.getData());
  
  if (enterBtnState == HIGH) {
    startGame();
    return;
  }
  
  delay(100);
}

void startGame() {
  currentScreen = 1;
  gameTimeStart = millis();
  gameTimeEnd = gameTimeStart + gameTime;

  statsShots = 0;
  statsDeath = 0;
  
  displayBasisInfo();
  calculateTime();
  displayTime();
}

void endGame() {
  currentScreen = 2;
}

void loopInGame() {
  if (gameTimeToEnd <= 0) {
    endGame();
    return;
  }
  
  colorWipe(strip.Color(0,0,intensity,0));
  
  calculateTime();
  displayTime();
  
  if (alive) {
    if (fireBtnState == HIGH) {
      unsigned long shotValue = infinitagCore.ir_encode(false, 0, playerTeamId, playerId, 1, 100);
      irsend.sendRC5(shotValue, 24);
      colorWipe(strip.Color(0,intensity,0,0));
      statsShots++;
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
  delay(100);
}

void getButtonStates() {
  rightBtnState = digitalRead(rightBtnPin);
  leftBtnState = digitalRead(leftBtnPin);
  downBtnState = digitalRead(downBtnPin);
  upBtnState = digitalRead(upBtnPin);
  specialBtnState = digitalRead(specialBtnPin);
  infoBtnState = digitalRead(infoBtnPin);
  reloadBtnState = digitalRead(reloadBtnPin);
  fireBtnState = digitalRead(fireBtnPin);
  enterBtnState = digitalRead(enterBtnPin);
  resetBtnState = digitalRead(resetBtnPin);
}

void demoFunktions() {
  if (upBtnState == HIGH) {
    if (playerId == 1) {
      playerId = 2;
    } else {
      playerId = 1;
      if (playerTeamId == 1) {
        playerTeamId = 2;
      } else {
        playerTeamId = 1;
      }
    }
    displayBasisInfo();
    updateSensorConfig();
    delay(100);
  }

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

void displayBasisInfo() {
  framebuffer.clear(BLACK);

  // Infinitag Smybol
  // Kann noch nicht richtig mit der aktuellen Lib abgebildet werden
  framebuffer.drawLine(25, 2, 48, 2, WHITE);
  framebuffer.drawLine(25, 49, 48, 49, WHITE);
  framebuffer.drawLine(10, 24, 10, 27, WHITE);
  
  framebuffer.drawLine(78, 2, 101, 2, WHITE);
  framebuffer.drawLine(78, 49, 101, 49, WHITE);
  framebuffer.drawLine(116, 24, 116, 27, WHITE);

  // Spieler Angabe
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  String displayPlayerText = "P";
  displayPlayerText += playerId;
  char charPlayerBuf[10];
  displayPlayerText.toCharArray(charPlayerBuf, 10);
  framebuffer.displayText(charPlayerBuf, 0, 0, WHITE);

  // Team Angabe
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  String displayTeamText = "T";
  displayTeamText += playerTeamId;
  char charTeamBuf[10];
  displayTeamText.toCharArray(charTeamBuf, 10);
  framebuffer.displayText(charTeamBuf, 112, 0, WHITE);

  display_buffer(&display, framebuffer.getData());

  displayTime();
}

void calculateTime() {
  gameTimeDiff = millis() - gameTimeStart;
  if (gameTimeDiff <= gameTime) {
    gameTimeToEnd = gameTime - gameTimeDiff;
    if (gameTimeToEnd > 0) {
      gameTimeDiffMinutes = (gameTimeToEnd / 60000);
      gameTimeDiffSeconds = (gameTimeToEnd - (gameTimeDiffMinutes * 60000)) / 1000;
      return;
    }
  }
  gameTimeToEnd = 0;
  gameTimeDiffMinutes = 0;
  gameTimeDiffSeconds = 0;
}

void displayTime() {
  
  // Bar
  int barMaxWidth = 94;
  framebuffer.drawHorizontalLine(0, 60, barMaxWidth, WHITE);
  framebuffer.drawLine(barMaxWidth, 60, barMaxWidth, 64, WHITE);

  framebuffer.drawRectFilled(0, 61, barMaxWidth, 4, BLACK);
  int barSize = barMaxWidth - (gameTimeDiff * barMaxWidth / gameTime);
  if (barSize < 0) {
    barSize = 0;
  }
  framebuffer.drawRectFilled(0, 61, barSize, 4, WHITE);

  // Time
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  framebuffer.drawRectFilled(97, 52, 31, 12, BLACK);
  String timeText = "";
  if (gameTimeDiffMinutes < 10) {
    timeText += "0";
  }
  timeText += gameTimeDiffMinutes;
  timeText += ":";
  if (gameTimeDiffSeconds < 10) {
    timeText += "0";
  }
  timeText += gameTimeDiffSeconds;
  char timeBuf[6];
  timeText.toCharArray(timeBuf, 6);
  framebuffer.displayText(timeBuf, 97, 52, WHITE);
  
  display_buffer(&display, framebuffer.getData());
}

void updateSensorConfig() {
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

