#pragma once

#include <Arduino.h>

// Structure partagée pour les mesures capteurs
struct SensorsData {
  float acc[3];
  float gyro[3];
  float mag[3];
  float lpsPressure;
  float lpsAlt;
  float bmpTemp;
  float bmpPressure;
  float bmpAlt;
};

// Données globales et mutex associés
extern SensorsData gSensors;
extern SemaphoreHandle_t mutexSensors;

