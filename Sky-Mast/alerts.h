/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Alerts, Safety Interlock & Actuator Control
 * File: alerts.h
 */

#ifndef ALERTS_H
#define ALERTS_H

#include <Actuator.h>
#include "utils.h"

// Multi-Pin Fail-Safe Buzzer Output (D4, D12, D13, D27)
#define BUZZER_PIN_D4  4
#define BUZZER_PIN_D12 12
#define BUZZER_PIN_D13 13
#define BUZZER_PIN_D27 27

class AlertController {
public:
    AlertController();
    bool begin();
    bool update(SystemState &state);
    bool isConnected();
    void triggerBeep(uint16_t durationMs);
    void triggerBuzzerBeeps(uint8_t count);
    void setRelayState(bool enable);
    void processCommandChar(char c);

private:
    Actuator _actuator;
    NonBlockingTimer _alarmBlinkTimer;
    NonBlockingTimer _multiBeepTimer;
    bool _initialized;
    bool _actuatorPresent;
    bool _toggleState;
    uint32_t _beepOffTime;
    uint8_t _remainingBeeps;
    bool _relayState;

    void setBuzzerTone(uint16_t freq);
    void stopBuzzerTone();
};

#endif // ALERTS_H
