#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <ArtnetWifi.h>
#include <esp_dmx.h>
#include <WiFiUdp.h>

// Config Net
const char* ssid = "iPhone Pepon";
const char* password = "1234qwer";
//const char* ssid = "Motomami";
//const char* password = "Soporte01";

#define DHCP_DISABLED

#ifdef DHCP_DISABLED
IPAddress local_IP(192, 168, 1, 154);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);  //optional
IPAddress secondaryDNS(1, 1, 1, 1);    //optional
#endif
TFT_eSPI tft = TFT_eSPI();

WiFiUDP UdpSend;
ArtnetWifi artnet;

//config ArtNet

int transmitPinA = 17;
int receivePinA = 16;  // Not connected
int enablePinA = 4;

int transmitPinB = 21;
int receivePinB = 16;  // Not connected
int enablePinB = 19;

dmx_port_t dmxPortA = 1;
dmx_port_t dmxPortB = 2;

byte dataA[DMX_PACKET_SIZE];
byte dataB[DMX_PACKET_SIZE];

const int startUniverse = 0;  
const int maxUniverses = 2;
const int numberOfChannels = 1024;
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;


#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

int x, y, z;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code( void * parameter ) { 
  for(;;) {  
    Serial.println("Task 1 is running");
    delay(1000);
  } 
}


void Task2code( void * parameter ) {
  for(;;) {  
    Serial.println("Task 2 is running");
    delay(2000);  
  }
}



void printTouchToSerial(int touchX, int touchY, int touchZ) {
  Serial.print("X = ");
  Serial.print(touchX);
  Serial.print(" | Y = ");
  Serial.print(touchY);
  Serial.print(" | Pressure = ");
  Serial.print(touchZ);
  Serial.println();
}


void printTouchToDisplay(int touchX, int touchY, int touchZ) {

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  int centerX = SCREEN_WIDTH / 2;
  int textY = 80;
 
  String tempText = "X = " + String(touchX);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  textY += 20;
  tempText = "Y = " + String(touchY);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  textY += 20;
  tempText = "Pressure = " + String(touchZ);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);
}

void setup() {
  Serial.begin(115200);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1);

  tft.fillScreen(TFT_BACKLIGHT_ON);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  //Config Wifi & init Arnet
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  artnet.setArtDmxCallback(onArtNetFrame);
  artnet.begin("ESP32-ArtNet-to-DMX-Converter");

  dmx_set_pin(dmxPortA, transmitPinA, receivePinA, enablePinA);
  dmx_set_pin(dmxPortB, transmitPinB, receivePinB, enablePinB);

  dmx_driver_install(dmxPortA, DMX_DEFAULT_INTR_FLAGS);
  dmx_driver_install(dmxPortB, DMX_DEFAULT_INTR_FLAGS);

  tft.drawCentreString("Hello, world!", centerX, 30, FONT_SIZE);
  tft.drawCentreString("Touch screen to test", centerX, centerY, FONT_SIZE);
}

void onArtNetFrame(uint16_t universe, uint16_t numberOfChannels, uint8_t sequence, uint8_t* dmxData) {
  sendFrame = 1;

  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++) {
    if (universesReceived[i] == 0) {
      sendFrame = 0;
      break;
    }
  }
  
  for (int i = 0; i < numberOfChannels; i++) {
    if (universe == startUniverse)
      dataA[i + 1] = dmxData[i];
    else if (universe == startUniverse + 1)
      dataB[i + 1] = dmxData[i];
  }

   
  previousDataLength = numberOfChannels;

  dmx_write(dmxPortA, dataA, DMX_MAX_PACKET_SIZE);
  dmx_write(dmxPortB, dataB, DMX_MAX_PACKET_SIZE);
  dmx_send(dmxPortA, DMX_PACKET_SIZE);
  dmx_send(dmxPortB, DMX_PACKET_SIZE);
  dmx_wait_sent(dmxPortA, DMX_TIMEOUT_TICK);
  dmx_wait_sent(dmxPortB, DMX_TIMEOUT_TICK);
  
  memset(universesReceived, 0, maxUniverses);
}

void loop() {
  
  
  if (touchscreen.tirqTouched() && touchscreen.touched()) {

    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    printTouchToSerial(x, y, z);
    printTouchToDisplay(x, y, z);

    delay(100);
  }

  if ((WiFi.status() == WL_CONNECTED)) {
    artnet.read();
  }
}
