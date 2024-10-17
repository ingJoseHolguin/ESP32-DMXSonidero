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


 
  

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  tft.drawCentreString("Hello, world!", centerX, 30, FONT_SIZE);
  tft.drawCentreString("Touch screen to test", centerX, centerY, FONT_SIZE);
}



byte count=1;

void loop() {

  if (Serial.available() > 0) {
    // Leemos el primer carácter ingresado
    char input = Serial.read();
    
    // Ejecutamos diferentes funciones dependiendo del carácter recibido
    if (input == 'r') {
      tft.fillRect(10, 10, 100, 50, ILI9341_RED);  
    };

    if (input == 'b') {
      tft.fillRect(10, 10, 100, 50, ILI9341_BLUE);  
    };

    if (input == 'g') {
      tft.fillRect(10, 10, 100, 50, ILI9341_GREEN);  
    };

    if (input == 'a') {
      tft.fillRect(10, 10, 100, 50, ILI9341_BLACK);  
    };

    if (input == 'w') {
      tft.fillRect(10, 10, 100, 50, ILI9341_WHITE);  
    };
  }
  
  
  if (touchscreen.tirqTouched() && touchscreen.touched()) {

    switch (count)
    {
    case 1:
      tft.fillRect(0, 0, 320, 240, ILI9341_RED);  
      break;
    case 2:
      tft.fillRect(0, 0, 320, 240, ILI9341_GREEN); 
      break;
    case 3:
      tft.fillRect(0, 0, 320, 240, ILI9341_BLUE); 
      break;
    case 4:
      tft.fillRect(0, 0, 320, 240, ILI9341_WHITE);
      break;
    case 5:
      tft.fillRect(0, 0, 320, 240, ILI9341_BLACK);
      break;
    
    default:
      count = 0;
      break;
    }   

    count++;

    delay(250);
  }

  if ((WiFi.status() == WL_CONNECTED)) {
    artnet.read();
  }
}
