#include "task_serial.h"

void taskSerial(void *pvParameters) {
  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      Serial.printf("[Serial] Reçu : '%c'\n", c);

      switch (c) {
        case 'a':
          // Notifie taskServos avec la valeur 'a'
          xTaskNotify(hTaskServos, (uint32_t)'a', eSetValueWithOverwrite);
          break;
        case 'd':
          xTaskNotify(hTaskServos, (uint32_t)'d', eSetValueWithOverwrite);
          break;
        // Ajoutez d'autres commandes ici
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Évite le busy-loop
  }
}