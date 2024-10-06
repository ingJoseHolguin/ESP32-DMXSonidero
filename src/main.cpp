#include <Arduino.h>
#include <TFT_eSPI.h>       // Librería para manejar la pantalla
#include <XPT2046_Touchscreen.h> // Librería para el touch capacitivo

#define TOUCH_CS 22         // Pin de Chip Select para el touch
#define TFT_CS 5            // Pin de Chip Select para la pantalla
#define TFT_RST 4           // Pin de reset de la pantalla
#define TFT_DC 2            // Pin de datos/comando de la pantalla

TFT_eSPI tft = TFT_eSPI();  // Instancia para la pantalla
XPT2046_Touchscreen ts(TOUCH_CS);  // Instancia para el touch}

TaskHandle_t Task1;
TaskHandle_t Task2;

// Función de la tarea 1, que será asignada al núcleo 0
void Task1code( void * parameter ) {
  Serial.print("Task 1 is running on core ");
  Serial.println(xPortGetCoreID());  // Imprime en qué núcleo está corriendo

  for(;;) {  // Bucle infinito de la tarea
    Serial.println("Task 1 is running");
    delay(1000);  // Espera 1 segundo
  }
}

// Función de la tarea 2, que será asignada al núcleo 1
void Task2code( void * parameter ) {
  Serial.print("Task 2 is running on core ");
  Serial.println(xPortGetCoreID());  // Imprime en qué núcleo está corriendo

  for(;;) {  // Bucle infinito de la tarea
    Serial.println("Task 2 is running");
    delay(2000);  // Espera 2 segundos
  }
}

void setup() {
  Serial.begin(115200);

    xTaskCreatePinnedToCore(
    Task1code,   // Función que ejecuta la tarea
    "Task1",     // Nombre de la tarea
    10000,       // Tamaño de la pila (en bytes)
    NULL,        // Parámetro para pasar a la tarea
    1,           // Prioridad de la tarea
    &Task1,      // Manejador de la tarea
    0);          // Núcleo 0

  // Crear la tarea 2 en el núcleo 1
  xTaskCreatePinnedToCore(
    Task2code,   // Función que ejecuta la tarea
    "Task2",     // Nombre de la tarea
    10000,       // Tamaño de la pila (en bytes)
    NULL,        // Parámetro para pasar a la tarea
    1,           // Prioridad de la tarea
    &Task2,      // Manejador de la tarea
    1);          // Núcleo 1


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