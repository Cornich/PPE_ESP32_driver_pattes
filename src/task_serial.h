#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


extern TaskHandle_t hTaskServos; // handle de la tâche à notifier
void taskSerial(void *pvParameters);    