#include <Arduino.h>

#include "sensors_shared.h"
#include "task_sensors.h"
#include "task_json.h"
#include "task_servos.h"
#include "task_serial.h"

void setup() {
  Serial.begin(115200);

  // Initialisation du matériel capteurs (I2C + IMU + BMP280)
  sensorsInit();

  // Création des tâches (comme pour la manette RTOS)
  xTaskCreatePinnedToCore(taskSensors, "Sensors", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(taskJson,    "JSON",    4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskServos,  "Servos",  4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskSerial,  "Serial",  2048, NULL, 1, NULL, 1); 

  //Serial.println(F("RTOS: tâches capteurs + JSON + servos"));
  Serial.println("Coucou voici le terminal");
}

void loop() {
  // Rien ici, tout se fait dans les tâches
  vTaskDelay(pdMS_TO_TICKS(1000));
}