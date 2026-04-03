#include <Arduino.h>
#include <cstdint>
#include "task_servos.h"

// === Configuration LEDC pour 8 servos ================================
// Pins PWM pour les 8 servos (à adapter selon ton câblage)
#define servoAvantDroiteHaut 0
#define servoAvantDroiteBas 1
#define servoAvantGaucheHaut 2
#define servoAvantGaucheBas 3
#define servoArriereDroiteHaut 4
#define servoArriereDroiteBas 5
#define servoArriereGaucheHaut 6
#define servoArriereGaucheBas  7
//position initiale
static const int8_t SERVO_PINS[8] = {13, 12, 14, 27, 26, 25, 33, 32};
static const int8_t SERVO_MIRROIR[8] = {1,1,-1,-1,1,1,-1,-1};

#include "positions.hpp"
// Paramètres LEDC
static const int PWM_FREQ      = 50;    // 50 Hz → période 20 ms
static const int PWM_RESOLUTION = 16;  // 16 bits → valeurs 0–80535

// Largeurs d'impulsion pour ESP32 LEDC (résolution 16 bits, freq 50 Hz)
// Période = 20 000 µs
// 0°  → ~500  µs → (500  / 20000) * 80535 ≈ 1638
// 50° → ~1166 µs → (1166 / 20000) * 80535 ≈ 3817
static const uint32_t PULSE_0DEG  = 1638;
static const uint32_t PULSE_50DEG = 3817;

// Utilitaire : convertit un angle (0–180°) en duty cycle 16 bits
static uint32_t angleToDuty(float angle) {
    // Impulsion entre 500 µs (0°) et 2500 µs (180°)
    float pulseUs = 500.0f + (angle / 180.0f) * 2000.0f;
    return (uint32_t)((pulseUs / 20000.0f) * 80535.0f);
}

float fmod180(float angle){
    return (angle<0?angle+180:(angle>180? angle-180 : angle));
}

static void setServo(uint8_t servo, float angle){
    uint32_t duty = angleToDuty(fmod180(angle*SERVO_MIRROIR[servo]));
    ledcWrite(servo, duty);   // canal i ↔ servo i
}

// Applique un angle à tous les servos
static void setAllServos(const float position[8]) {
    //uint32_t duty = angleToDuty(angle);
    for (uint8_t i = 0; i < 8; i++) {
        setServo(i, position[i]);
        //ledcWrite(i, duty);   // canal i ↔ servo i
    }
}

// === TÂCHE SERVOS ====================================================
void taskServos(void *pvParameters) {

    // --- Initialisation LEDC ---
    for (uint8_t i = 0; i < 8; i++) {
        ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);   // canal i, 50 Hz, 16 bits
        ledcAttachPin(SERVO_PINS[i], i);           // attache la pin au canal
    }

    // Départ : tous les servos à 0°
    setAllServos(positionDef);

    for (;;) {    
        
        for(int i=0;i<12;i++){
            setAllServos(positionMar[i]);
            vTaskDelay(pdMS_TO_TICKS(700));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
