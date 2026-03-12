#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <LSM6.h>
#include <LIS3MDL.h>
#include <LPS.h>
#include <ArduinoJson.h>

// --- Instances des capteurs ---
Adafruit_BMP280 bmp;
LSM6 imu;
LIS3MDL mag;
LPS ps;

void setup() {
  Serial.begin(115200);
  // I2C sur ESP32, câblé SDA=4, SCL=5 dans ton projet
  Wire.begin(4, 5);

  // Initialisation des capteurs
  bmp.begin(0x76);
  if (!imu.init() || !ps.init()) {
    Serial.println("Erreur de détection de l'AltIMU !");
    while (1);
  }
  imu.enableDefault();
  mag.init(); mag.enableDefault();
  ps.enableDefault();

  // --- CONFIGURATION DU FILTRAGE INTERNE ---

  // 1. Accéléromètre : ODR à ~104 Hz, LPF actif (exemple générique)
  // Registre CTRL1_XL : la valeur exacte dépend de ta révision de LSM6,
  // 0x40 correspond à 104 Hz, +/-2g, filtre anti‑aliasing typique.
  imu.writeReg(LSM6::CTRL1_XL, 0x40);

  // 2. Gyroscope : activation du filtre passe‑bas numérique (exemple neutre)
  imu.writeReg(LSM6::CTRL6_C, 0x00);

  // 3. Baromètre LPS : moyennage interne fort pour lisser la pression
  // 0x0F = moyenne sur 32 mesures pression / 16 température (selon datasheet)
  ps.writeReg(LPS::RES_CONF, 0x0F);

  Serial.println("\n--- Systeme Capteurs (IMU + BMP280) Pret ---");
}

void loop() {
  // --- LECTURE DE TOUS LES CAPTEURS ---
  imu.read();
  mag.read();

  // LPS (baromètre IMU)
  float pressionLps = ps.readPressureMillibars();
  float altitudeLps = ps.pressureToAltitudeMeters(pressionLps);

  // BMP280
  float tempBmp      = bmp.readTemperature();
  float pressionBmp  = bmp.readPressure() / 100.0f; // hPa
  float altitudeBmp  = bmp.readAltitude(1013.25);   // altitude estimée

  // --- CONSTRUCTION DU JSON PAR COMPOSANT ---
  JsonDocument doc;

  // Bloc IMU (AltIMU: LSM6 + LIS3MDL + LPS)
  JsonObject imuObj = doc["imu"].to<JsonObject>();

  JsonObject acc = imuObj["accel"].to<JsonObject>();
  acc["x"] = imu.a.x;
  acc["y"] = imu.a.y;
  acc["z"] = imu.a.z;

  JsonObject gyr = imuObj["gyro"].to<JsonObject>();
  gyr["x"] = imu.g.x;
  gyr["y"] = imu.g.y;
  gyr["z"] = imu.g.z;

  JsonObject mgn = imuObj["mag"].to<JsonObject>();
  mgn["x"] = mag.m.x;
  mgn["y"] = mag.m.y;
  mgn["z"] = mag.m.z;

  JsonObject lpsObj = imuObj["baro"].to<JsonObject>();
  lpsObj["pressure"] = pressionLps;
  lpsObj["altitude"] = altitudeLps;

  // Bloc BMP280
  JsonObject bmpObj = doc["bmp"].to<JsonObject>();
  bmpObj["temp"]      = tempBmp;
  bmpObj["pressure"]  = pressionBmp;
  bmpObj["altitude"]  = altitudeBmp;

  // Envoi du JSON complet sur le port série
  serializeJson(doc, Serial);
  Serial.println();
  Serial.flush();

  delay(100);
}