#include <Arduino.h>
#include <TFT_eSPI.h>       // Librería para manejar la pantalla
#include <XPT2046_Touchscreen.h> // Librería para el touch capacitivo

#define TOUCH_CS 22         // Pin de Chip Select para el touch
#define TFT_CS 5            // Pin de Chip Select para la pantalla
#define TFT_RST 4           // Pin de reset de la pantalla
#define TFT_DC 2            // Pin de datos/comando de la pantalla

TFT_eSPI tft = TFT_eSPI();  // Instancia para la pantalla
XPT2046_Touchscreen ts(TOUCH_CS);  // Instancia para el touch

void setup() {
  Serial.begin(115200);

  // Inicializa la pantalla
  tft.init();
  tft.setRotation(1);  // Configura la orientación de la pantalla

  // Inicializa el touch
  if (ts.begin()) {
    Serial.println("Touchscreen initialized");
  } else {
    Serial.println("Touchscreen initialization failed");
  }

  // Limpiar la pantalla
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  // Verificar si se toca la pantalla
  if (ts.touched()) {
    TS_Point p = ts.getPoint();  // Obtener la posición del touch

    // Muestra las coordenadas del toque
    Serial.print("Touch at: ");
    Serial.print(p.x);
    Serial.print(", ");
    Serial.println(p.y);

    // Dibuja un círculo donde se detectó el toque
    tft.fillCircle(p.x, p.y, 5, TFT_RED);
  }
}