#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <LSM6.h>
#include <LIS3MDL.h>
#include <LPS.h>

#include "sensors_shared.h"
#include "task_sensors.h"

// --- Instances des capteurs ---
static Adafruit_BMP280 bmp;
static LSM6 imu;
static LIS3MDL mag;
static LPS ps;

void sensorsInit() {
  // I2C sur ESP32, câblé SDA=4, SCL=5 dans ton projet
  Wire.begin(4, 5);

  // Initialisation des capteurs
  bmp.begin(0x76);
  if (!imu.init() || !ps.init()) {
    Serial.println("Erreur de détection de l'AltIMU !");
    while (1) {
      delay(100);
    }
  }
  imu.enableDefault();
  mag.init();
  mag.enableDefault();
  ps.enableDefault();

  // --- CONFIG LSM6DS33 (acc + gyro) ---
  // CTRL1_XL (0x10) : ODR_XL=104 Hz, FS_XL=±2 g, BW_XL=50 Hz -> 0x43
  imu.writeReg(LSM6::CTRL1_XL, 0x43);
  // CTRL2_G (0x11) : ODR_G=104 Hz, FS_G=500 dps -> 0x44
  imu.writeReg(LSM6::CTRL2_G, 0x44);
  // CTRL3_C (0x12) : BDU=1, IF_INC=1 -> 0x44
  imu.writeReg(LSM6::CTRL3_C, 0x44);
  // CTRL4_C (0x13) : XL_BW_SCAL_ODR=1 (BW_XL actif) -> 0x80
  imu.writeReg(LSM6::CTRL4_C, 0x80);
  // CTRL6_C (0x15) : XL_HM_MODE=0 (high performance) -> 0x00
  imu.writeReg(LSM6::CTRL6_C, 0x00);
  // CTRL7_G (0x16) : HPF gyro activé, coupure très basse -> 0x40
  imu.writeReg(LSM6::CTRL7_G, 0x40);

  // --- CONFIG LIS3MDL (magnétomètre) ---
  // CTRL_REG1 (0x20) : ODR=40 Hz, X/Y en high-performance, temp off -> 0x58
  mag.writeReg(LIS3MDL::CTRL_REG1, 0x58);
  // CTRL_REG2 (0x21) : FS = ±4 gauss -> 0x00
  mag.writeReg(LIS3MDL::CTRL_REG2, 0x00);
  // CTRL_REG3 (0x22) : mode continu, 4-wire SPI/I2C -> 0x00
  mag.writeReg(LIS3MDL::CTRL_REG3, 0x00);
  // CTRL_REG4 (0x23) : Z en high-performance -> 0x08
  mag.writeReg(LIS3MDL::CTRL_REG4, 0x08);
  // CTRL_REG5 (0x24) : BDU=1 -> 0x40
  mag.writeReg(LIS3MDL::CTRL_REG5, 0x40);

  // --- CONFIG LPS25H (baromètre) ---
  // RES_CONF (0x10) : moyennage interne max -> 0x0F
  ps.writeReg(LPS::RES_CONF, 0x0F);
  // CTRL_REG1 (0x20) : ODR ≈ 7 Hz, BDU=1, PD=1 -> 0xAA
  ps.writeReg(LPS::CTRL_REG1, 0xAA);

  // Mutex pour protéger les données partagées
  mutexSensors = xSemaphoreCreateMutex();
  if (mutexSensors == nullptr) {
    Serial.println("Erreur creation mutexSensors");
  }

  Serial.println(F("--- Capteurs (IMU + BMP280) initialisés ---"));
}

// === TÂCHE CAPTEURS ==================================================
void taskSensors(void *pvParameters) {
  const TickType_t period = pdMS_TO_TICKS(10); // 100 Hz
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    imu.read();
    mag.read();
    float pLps = ps.readPressureMillibars();
    float altLps = ps.pressureToAltitudeMeters(pLps);
    float tBmp = bmp.readTemperature();
    float pBmp = bmp.readPressure() / 100.0f;
    float altBmp = bmp.readAltitude(1013.25);

    if (mutexSensors != nullptr &&
        xSemaphoreTake(mutexSensors, pdMS_TO_TICKS(1)) == pdTRUE) {
      gSensors.acc[0] = imu.a.x;
      gSensors.acc[1] = imu.a.y;
      gSensors.acc[2] = imu.a.z;
      gSensors.gyro[0] = imu.g.x;
      gSensors.gyro[1] = imu.g.y;
      gSensors.gyro[2] = imu.g.z;
      gSensors.mag[0] = mag.m.x;
      gSensors.mag[1] = mag.m.y;
      gSensors.mag[2] = mag.m.z;
      gSensors.lpsPressure = pLps;
      gSensors.lpsAlt = altLps;
      gSensors.bmpTemp = tBmp;
      gSensors.bmpPressure = pBmp;
      gSensors.bmpAlt = altBmp;
      xSemaphoreGive(mutexSensors);
    }

    vTaskDelayUntil(&lastWake, period);
  }
}

