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
  // CTRL5_C (0x14) : arrondi acc + gyro pour réduire le bruit LSB (Table 55: 011)
  imu.writeReg(LSM6::CTRL5_C, 0x03);
  // CTRL6_C (0x15) : XL_HM_MODE=0 (high performance) -> 0x00
  imu.writeReg(LSM6::CTRL6_C, 0x00);
  // CTRL7_G (0x16) : HPF gyro activé, coupure très basse -> 0x40
  imu.writeReg(LSM6::CTRL7_G, 0x40);

  // --- CONFIG LIS3MDL (magnétomètre) ---
  // CTRL_REG1 (0x20) : ODR=20 Hz (au lieu de 40), X/Y high-perf, temp off -> moins de bruit
  mag.writeReg(LIS3MDL::CTRL_REG1, 0x50);
  // CTRL_REG2 (0x21) : FS = ±4 gauss -> 0x00
  mag.writeReg(LIS3MDL::CTRL_REG2, 0x00);
  // CTRL_REG3 (0x22) : mode continu, 4-wire -> 0x00
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

  // --- AUTRES OPTIONS REGISTRES (si besoin d'aller plus loin) ---
  // LSM6 : ODR 52 Hz au lieu de 104 (CTRL1_XL 0x33, CTRL2_G 0x33) = encore plus lisse.
  // LSM6 : BW_XL 100 Hz au lieu de 50 (CTRL1_XL 0x42) = moins lisse, plus réactif.
  // LIS3MDL : ODR 10 Hz (CTRL_REG1 0x48) = mag très lisse, cap plus lent.
  // LPS25H : pas d'autre reg de filtrage utile ; RES_CONF=0x0F est déjà le max.

  // Mutex pour protéger les données partagées
  mutexSensors = xSemaphoreCreateMutex();
  if (mutexSensors == nullptr) {
    Serial.println("Erreur creation mutexSensors");
  }

  Serial.println(F("--- Capteurs (IMU + BMP280) initialisés ---"));
}

// === TÂCHE CAPTEURS ==================================================
// Filtre exponentiel (alpha petit = plus lisse, plus de lag)
static const float ALPHA_ACC  = 0.25f;
static const float ALPHA_GYRO = 0.25f;
static const float ALPHA_MAG   = 0.2f;
static const float ALPHA_BARO = 0.3f;

void taskSensors(void *pvParameters) {
  const TickType_t period = pdMS_TO_TICKS(10); // 100 Hz
  TickType_t lastWake = xTaskGetTickCount();
  static bool firstRun = true;
  static SensorsData filt;

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
      // Premier échantillon : initialiser le filtre
      if (firstRun) {
        filt.acc[0] = imu.a.x;
        filt.acc[1] = imu.a.y;
        filt.acc[2] = imu.a.z;
        filt.gyro[0] = imu.g.x;
        filt.gyro[1] = imu.g.y;
        filt.gyro[2] = imu.g.z;
        filt.mag[0] = mag.m.x;
        filt.mag[1] = mag.m.y;
        filt.mag[2] = mag.m.z;
        filt.lpsPressure = pLps;
        filt.lpsAlt = altLps;
        filt.bmpTemp = tBmp;
        filt.bmpPressure = pBmp;
        filt.bmpAlt = altBmp;
        firstRun = false;
      } else {
        // Filtre exponentiel : out = out + alpha * (raw - out)
        filt.acc[0] += ALPHA_ACC  * (imu.a.x   - filt.acc[0]);
        filt.acc[1] += ALPHA_ACC  * (imu.a.y   - filt.acc[1]);
        filt.acc[2] += ALPHA_ACC  * (imu.a.z   - filt.acc[2]);
        filt.gyro[0] += ALPHA_GYRO * (imu.g.x   - filt.gyro[0]);
        filt.gyro[1] += ALPHA_GYRO * (imu.g.y   - filt.gyro[1]);
        filt.gyro[2] += ALPHA_GYRO * (imu.g.z   - filt.gyro[2]);
        filt.mag[0] += ALPHA_MAG   * (mag.m.x   - filt.mag[0]);
        filt.mag[1] += ALPHA_MAG   * (mag.m.y   - filt.mag[1]);
        filt.mag[2] += ALPHA_MAG   * (mag.m.z   - filt.mag[2]);
        filt.lpsPressure += ALPHA_BARO * (pLps   - filt.lpsPressure);
        filt.lpsAlt += ALPHA_BARO * (altLps - filt.lpsAlt);
        filt.bmpTemp += ALPHA_BARO * (tBmp - filt.bmpTemp);
        filt.bmpPressure += ALPHA_BARO * (pBmp - filt.bmpPressure);
        filt.bmpAlt += ALPHA_BARO * (altBmp - filt.bmpAlt);
      }
      gSensors = filt;
      xSemaphoreGive(mutexSensors);
    }

    vTaskDelayUntil(&lastWake, period);
  }
}

