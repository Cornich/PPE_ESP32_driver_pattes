#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <LSM6.h>
#include <LIS3MDL.h>
#include <LPS.h>
#include <ArduinoJson.h>


// Raspberry PI 5 : Robot : MAC : 88:a2:9e:5e:8e:24

// --- Instances des capteurs ---
Adafruit_BMP280 bmp;
LSM6 imu;
LIS3MDL mag;
LPS ps;

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5); // SDA=D2, SCL=D1

  // Initialisation des capteurs
  bmp.begin(0x76);
  imu.init(); imu.enableDefault();
  mag.init(); mag.enableDefault();
  ps.init();  ps.enableDefault();

  Serial.println("\n--- Systeme Capteurs Pret ---");
}

void loop() {
  // --- LECTURE DE TOUS LES CAPTEURS ---
  imu.read();
  mag.read();
  float pressionLps = ps.readPressureMillibars();
  float altitudeLps = ps.pressureToAltitudeMeters(pressionLps);

  // --- CONSTRUCTION DU JSON CAPTEURS ---
  JsonDocument doc;

  // Environnement (BMP280 + LPS)
  doc["temp"] = bmp.readTemperature();
  doc["pression_bmp"] = bmp.readPressure() / 100.0;
  doc["pression_lps"] = pressionLps;
  doc["alt_lps"] = altitudeLps;

  // Accéléromètre (LSM6)
  JsonObject acc = doc["accel"].to<JsonObject>();
  acc["x"] = imu.a.x; acc["y"] = imu.a.y; acc["z"] = imu.a.z;

  // Gyroscope (LSM6)
  JsonObject gyr = doc["gyro"].to<JsonObject>();
  gyr["x"] = imu.g.x; gyr["y"] = imu.g.y; gyr["z"] = imu.g.z;

  // Magnétomètre (LIS3MDL)
  JsonObject mgn = doc["mag"].to<JsonObject>();
  mgn["x"] = mag.m.x; mgn["y"] = mag.m.y; mgn["z"] = mag.m.z;

  // Envoi
  serializeJson(doc, Serial);
  Serial.println(); 
  Serial.flush();
  
  delay(100); 
}