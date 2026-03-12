#pragma once

// Initialisation du bus I2C et des capteurs IMU + BMP280
void sensorsInit();

// Tâche RTOS de lecture des capteurs
void taskSensors(void *pvParameters);

