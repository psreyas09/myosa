/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Alerts, Safety Interlock & Actuator Control
 * File: alerts.cpp
 */

#include "alerts.h"

AlertController::AlertController()
    : _alarmBlinkTimer(140),
      _multiBeepTimer(100),
      _initialized(false),
      _actuatorPresent(false),
      _toggleState(false),
      _beepOffTime(0),
      _remainingBeeps(0),
      _relayState(true) {
}

void AlertController::setBuzzerTone(uint16_t freq) {
    tone(BUZZER_PIN_D4, freq);
    tone(BUZZER_PIN_D12, freq);
    tone(BUZZER_PIN_D13, freq);
    tone(BUZZER_PIN_D27, freq);

    digitalWrite(BUZZER_PIN_D4, HIGH);
    digitalWrite(BUZZER_PIN_D12, HIGH);
    digitalWrite(BUZZER_PIN_D13, HIGH);
    digitalWrite(BUZZER_PIN_D27, HIGH);
}

void AlertController::stopBuzzerTone() {
    noTone(BUZZER_PIN_D4);
    noTone(BUZZER_PIN_D12);
    noTone(BUZZER_PIN_D13);
    noTone(BUZZER_PIN_D27);

    // Re-assert pinMode OUTPUT to un-route ESP32 LEDC PWM matrix channel
    pinMode(BUZZER_PIN_D4, OUTPUT);
    pinMode(BUZZER_PIN_D12, OUTPUT);
    pinMode(BUZZER_PIN_D13, OUTPUT);
    pinMode(BUZZER_PIN_D27, OUTPUT);

    digitalWrite(BUZZER_PIN_D4, LOW);
    digitalWrite(BUZZER_PIN_D12, LOW);
    digitalWrite(BUZZER_PIN_D13, LOW);
    digitalWrite(BUZZER_PIN_D27, LOW);
}

bool AlertController::begin() {
    pinMode(BUZZER_PIN_D4, OUTPUT);
    pinMode(BUZZER_PIN_D12, OUTPUT);
    pinMode(BUZZER_PIN_D13, OUTPUT);
    pinMode(BUZZER_PIN_D27, OUTPUT);

    stopBuzzerTone();

    _actuatorPresent = _actuator.ping();
    if (_actuatorPresent) {
        _actuator.setMode(AC_SWITCH_IO, IO_OUTPUT);
        _actuator.setMode(BUZZER_IO, IO_OUTPUT);
        _actuator.setMode(IO2, IO_OUTPUT);
        _actuator.setMode(IO3, IO_OUTPUT);

        _actuator.setState(AC_SWITCH_IO, IO_HIGH); // Enable crane circuit on boot
        _actuator.setState(BUZZER_IO, IO_LOW);
        _actuator.setState(IO2, IO_LOW);
        _actuator.setState(IO3, IO_LOW);
    }

    _initialized = true;
    return true;
}

bool AlertController::isConnected() {
    return true;
}

void AlertController::triggerBeep(uint16_t durationMs) {
    setBuzzerTone(2500);
    if (_actuatorPresent) {
        _actuator.setState(BUZZER_IO, IO_HIGH);
    }
    _beepOffTime = millis() + durationMs;
}

void AlertController::triggerBuzzerBeeps(uint8_t count) {
    _remainingBeeps = count * 2; // ON and OFF phases
}

void AlertController::setRelayState(bool enable) {
    _relayState = enable;
    if (_actuatorPresent) {
        _actuator.setState(AC_SWITCH_IO, enable ? IO_HIGH : IO_LOW);
    }
}

void AlertController::processCommandChar(char c) {
    if (c == 'w' || c == 'W') return;

    Serial.print("MYOSA Command Received: ");
    Serial.println(c);

    switch (c) {
        case 'r':
        case 'R':
            setRelayState(true);
            break;
        case 'c':
        case 'C':
            setRelayState(false);
            break;
        case 'b':
        case 'B':
            triggerBuzzerBeeps(3);
            break;
        default:
            break;
    }
}

bool AlertController::update(SystemState &state) {
    _actuatorPresent = _actuator.ping();
    state.sensorStatus.actuatorOk = true;

    // Check severe tilt / deflection warning (>15 deg total deflection or >45 deg tilt)
    if (state.sensorStatus.imuOk) {
        if (state.orientation.totalDeflection > 15.0f || 
            fabsf(state.orientation.tiltX) > 45.0f || 
            fabsf(state.orientation.tiltY) > 45.0f) {
            state.highTiltWarning = true;
        } else {
            state.highTiltWarning = false;
        }
    } else {
        state.highTiltWarning = false;
    }

    // Determine if any critical safety hazard is active
    bool hazardActive = (state.structure.resonanceHazard || 
                         state.wind.highWindWarning || 
                         state.highTiltWarning);

    // Autonomous Safety Interlock Action:
    if (hazardActive) {
        state.mastInterlockEngaged = false; // Interlock TRIP
        if (_actuatorPresent) {
            _actuator.setState(AC_SWITCH_IO, IO_LOW); // Cut crane power
        }

        // Pulse audio siren
        if (_alarmBlinkTimer.isReady()) {
            _toggleState = !_toggleState;
            if (_toggleState) {
                uint16_t freq = 2600;
                if (state.structure.resonanceHazard) freq = 2900;
                else if (state.wind.highWindWarning) freq = 2300;
                setBuzzerTone(freq);
            } else {
                stopBuzzerTone();
            }

            if (_actuatorPresent) {
                _actuator.setState(BUZZER_IO, _toggleState ? IO_HIGH : IO_LOW);
            }
        }
        return true;
    } else {
        state.mastInterlockEngaged = true; // Normal operational state
        if (_actuatorPresent) {
            _actuator.setState(AC_SWITCH_IO, _relayState ? IO_HIGH : IO_LOW);
        }

        // Handle one-shot beeps
        if (_beepOffTime > 0 && millis() >= _beepOffTime) {
            _beepOffTime = 0;
            if (_remainingBeeps == 0) {
                stopBuzzerTone();
                if (_actuatorPresent) _actuator.setState(BUZZER_IO, IO_LOW);
            }
        }

        // Handle multi-beep sequencer
        if (_remainingBeeps > 0 && _multiBeepTimer.isReady()) {
            _remainingBeeps--;
            if (_remainingBeeps % 2 != 0) {
                setBuzzerTone(2500);
                if (_actuatorPresent) _actuator.setState(BUZZER_IO, IO_HIGH);
            } else {
                stopBuzzerTone();
                if (_actuatorPresent) _actuator.setState(BUZZER_IO, IO_LOW);
            }
        }

        if (_beepOffTime == 0 && _remainingBeeps == 0) {
            stopBuzzerTone();
            if (_actuatorPresent) {
                _actuator.setState(BUZZER_IO, IO_LOW);
            }
        }
    }

    return false;
}
