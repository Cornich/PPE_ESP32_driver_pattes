#include <Arduino.h>

#include "task_servos.h"

// === TÂCHE SERVOS (squelette pour 8 PWM) =============================
// À compléter : configuration LEDC + gestion d'une file de commandes.
void taskServos(void *pvParameters) {
  for (;;) {
    // Exemple : en attendant la vraie logique de servos
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

