#include <Arduino.h>
#include "sensors_shared.h"

// Définition des variables globales déclarées dans le header
SensorsData gSensors;
SemaphoreHandle_t mutexSensors = nullptr;

