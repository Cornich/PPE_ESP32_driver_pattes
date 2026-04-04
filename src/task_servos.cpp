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
static const int8_t SERVO_PINS[8] = {13, 12, 14, 27, 26, 25, 33, 32};
static const int8_t SERVO_MIRROIR[8] = {1,1,-1,-1,1,1,-1,-1};
//pour que l'envoie d'un angle fasse partir tous les servos dans la même direction
#include "positions.hpp"
float decDrt=0, decGch=0, decAvt=0, decArr=0;//décallage pour lever ou baisse un côté du robot
// Paramètres LEDC
static const int PWM_FREQ      = 50;    // 50 Hz → période 20 ms
static const int PWM_RESOLUTION = 16;  // 16 bits → valeurs 0–65535

// Largeurs d'impulsion pour ESP32 LEDC (résolution 16 bits, freq 50 Hz)
// Période = 20 000 µs
// 0°  → ~500  µs → (500  / 20000) * 65535 ≈ 1638
// 50° → ~1166 µs → (1166 / 20000) * 65535 ≈ 3817
static const uint32_t PULSE_0DEG  = 1638;
static const uint32_t PULSE_50DEG = 3817;

TaskHandle_t hTaskServos = NULL; // définition globale

float appliquerDecal(int i){
    float decallage=0;
    if(i<4)decallage+=decAvt;
    else decallage+=decArr;
    if(i==0 || i==1 || i==4 || i==5) decallage+=decDrt;
    else decallage+=decGch;
    if(i==1 || i==3 || i==5 || i==7) decallage= -decallage;
    return decallage;
}
// Utilitaire : convertit un angle (0–180°) en duty cycle 16 bits
static uint32_t angleToDuty(float angle) {
    // Impulsion entre 500 µs (0°) et 2500 µs (180°)
    float pulseUs = 500.0f + (angle / 180.0f) * 2000.0f;
    return (uint32_t)((pulseUs / 20000.0f) * 65535.0f);
}
float fmod180(float angle){//retourne un modulo 180 (différence avec fmod: prend des float)
    return (angle<0?angle+180:(angle>180? angle-180 : angle));
}
static void setServo(uint8_t servo, float angle){
    uint32_t duty = angleToDuty(fmod180(angle*SERVO_MIRROIR[servo]));
    ledcWrite(servo, duty);   // canal i ↔ servo i
}
// Applique une position
static void setAllServos(const float position[8]) {
    //uint32_t duty = angleToDuty(angle);
    for (uint8_t i = 0; i < 8; i++) {
        setServo(i, position[i]/*+appliquerDecal(i)*/);
        //ledcWrite(i, duty);   // canal i ↔ servo i
    }
}

void avancer(){
    for(int i=0;i<nbPosAvancer;i++){
        setAllServos(positionMar[i]);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
void setPositionDef(){
    setAllServos(position090);
}
// === TÂCHE SERVOS ====================================================
void taskServos(void *pvParameters) {
    hTaskServos = xTaskGetCurrentTaskHandle(); // enregistre le handle

    uint32_t cmd;
    // --- Initialisation LEDC ---
    for (uint8_t i = 0; i < 8; i++) {
        ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);   // canal i, 50 Hz, 16 bits
        ledcAttachPin(SERVO_PINS[i], i);           // attache la pin au canal
    }

    // Départ : tous les servos à 0°
    setAllServos(positionBas );

    for (;;) {    
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &cmd, portMAX_DELAY)) {
            Serial.printf("[Servos] Commande reçue : '%c'\n", (char)cmd);
            switch ((char)cmd) {
                case 'a': {
                    avancer();
                    break;
                }
                case 'd': {
                    setPositionDef();
                    break;
                };
            }
      }
       vTaskDelay(pdMS_TO_TICKS(100)); // Évite le busy-loop 
    }
}