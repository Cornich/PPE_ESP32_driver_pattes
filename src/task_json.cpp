#include <Arduino.h>
#include <ArduinoJson.h>

#include "sensors_shared.h"
#include "task_json.h"

// === TÂCHE JSON / ENVOI ==============================================
void taskJson(void *pvParameters) {
  const TickType_t period = pdMS_TO_TICKS(50); // 20 Hz
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    SensorsData snap;

    if (mutexSensors != nullptr &&
        xSemaphoreTake(mutexSensors, pdMS_TO_TICKS(5)) == pdTRUE) {
      snap = gSensors;  // copie locale
      xSemaphoreGive(mutexSensors);
    } else {
      vTaskDelayUntil(&lastWake, period);
      continue;
    }
/*
    JsonDocument doc;

    // Bloc IMU (AltIMU: LSM6 + LIS3MDL + LPS)
    JsonObject imuObj = doc["imu"].to<JsonObject>();

    JsonObject acc = imuObj["accel"].to<JsonObject>();
    acc["x"] = snap.acc[0];
    acc["y"] = snap.acc[1];
    acc["z"] = snap.acc[2];

    JsonObject gyr = imuObj["gyro"].to<JsonObject>();
    gyr["x"] = snap.gyro[0];
    gyr["y"] = snap.gyro[1];
    gyr["z"] = snap.gyro[2];

    JsonObject mgn = imuObj["mag"].to<JsonObject>();
    mgn["x"] = snap.mag[0];
    mgn["y"] = snap.mag[1];
    mgn["z"] = snap.mag[2];

    JsonObject lpsObj = imuObj["baro"].to<JsonObject>();
    lpsObj["pressure"] = snap.lpsPressure;
    lpsObj["altitude"] = snap.lpsAlt;

    // Bloc BMP280
    JsonObject bmpObj = doc["bmp"].to<JsonObject>();
    bmpObj["temp"] = snap.bmpTemp;
    bmpObj["pressure"] = snap.bmpPressure;
    bmpObj["altitude"] = snap.bmpAlt;

    serializeJson(doc, Serial);*/
    Serial.println();

    Serial.print(">acc_x:"); Serial.println(snap.acc[0]);
    Serial.print(">acc_y:"); Serial.println(snap.acc[1]);
    Serial.print(">acc_z:"); Serial.println(snap.acc[2]);
    Serial.print(">gyro_x:"); Serial.println(snap.gyro[0]);
    Serial.print(">gyro_y:"); Serial.println(snap.gyro[1]);
    Serial.print(">gyro_z:"); Serial.println(snap.gyro[2]);
    Serial.print(">mag_x:");  Serial.println(snap.mag[0]);
    Serial.print(">mag_y:");  Serial.println(snap.mag[1]);
    Serial.print(">mag_z:");  Serial.println(snap.mag[2]);
    Serial.print(">bmp_temp:"); Serial.println(snap.bmpTemp);
    Serial.print(">bmp_p:");   Serial.println(snap.bmpPressure);
    Serial.print(">lps_p:");   Serial.println(snap.lpsPressure);
    Serial.print(">lps_alt:"); Serial.println(snap.lpsAlt);

    Serial.flush();

    vTaskDelayUntil(&lastWake, period);
  }
}

