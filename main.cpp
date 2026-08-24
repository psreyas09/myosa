/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Main Application Controller
 * File: main.cpp
 */

#include <Arduino.h>
#include <Wire.h>

#include "utils.h"
#include "display.h"
#include "imu.h"
#include "barometer.h"
#include "gesture.h"
#include "alerts.h"
#include "bt.h"

// MYOSA Onboard Tactile Button (BOOT / USER Button on GPIO 0)
#define ONBOARD_BUTTON_PIN 0

// Global System Objects
SystemState g_systemState;
DisplayController g_display;
IMUController g_imu;
BarometerController g_baro;
GestureController g_gesture;
AlertController g_alerts;
BTController g_bt;

void skyMastSetup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=======================================================");
    Serial.println("  SKY-MAST: SMART CRANE & STRUCTURE SAFETY AUDITOR     ");
    Serial.println("=======================================================");

    // Initialize Onboard Button (GPIO 0 with Internal Pullup)
    pinMode(ONBOARD_BUTTON_PIN, INPUT_PULLUP);

    // Initialize I2C Bus at 400 kHz Fast Mode for high-speed IMU & OLED transfers
    Wire.begin();
    Wire.setClock(400000);

    // Step 1: Initialize OLED Display
    Serial.print("Initializing OLED Display... ");
    bool displayOk = g_display.begin();
    if (displayOk) {
        Serial.println("OK");
        g_display.showStartupScreen();
    } else {
        Serial.println("FAILED!");
    }

    // Step 2: Initialize Sensors & Processing Engines
    Serial.print("Initializing MPU6050 100Hz IMU & FFT Vibration Engine... ");
    g_systemState.sensorStatus.imuOk = g_imu.begin();
    Serial.println(g_systemState.sensorStatus.imuOk ? "OK" : "ERROR / NOT FOUND");

    Serial.print("Initializing BMP180 Bernoulli Fluid Dynamics Barometer... ");
    g_systemState.sensorStatus.baroOk = g_baro.begin();
    Serial.println(g_systemState.sensorStatus.baroOk ? "OK" : "ERROR / NOT FOUND");

    Serial.print("Initializing Gesture Fallback Controller... ");
    g_systemState.sensorStatus.gestureOk = g_gesture.begin();
    Serial.println("BYPASS / MANUAL BUTTON MODE");

    Serial.print("Initializing PCA9536 Safety Interlock & Actuator Board... ");
    g_systemState.sensorStatus.actuatorOk = g_alerts.begin();
    Serial.println(g_systemState.sensorStatus.actuatorOk ? "OK" : "STANDBY / DIRECT GPIO");

    // Step 3: Initialize Bluetooth SPP Telemetry
    Serial.print("Initializing MYOSA Bluetooth SPP Telemetry (MYOSA_1)... ");
    g_systemState.bleActive = g_bt.begin("MYOSA_1");
    Serial.println(g_systemState.bleActive ? "OK (Broadcasting)" : "FAILED");

    // Step 4: Begin Startup Tap-Test Calibration Phase
    g_systemState.mode = MODE_CALIBRATING;
    g_systemState.calibrationStartMs = millis();
    g_systemState.calibrationDurationMs = 5000; // 5.0 seconds window
    g_systemState.structure.resonantFreqHz = 12.5f; // Default baseline frequency
    g_systemState.autoCyclePages = false; // Default: Manual page control via board button

    // Play startup chirp
    g_alerts.triggerBeep(120);

    Serial.println("\n[SYSTEM] Entering 5.0s Tap-Test Calibration Mode...");
    Serial.println("[SYSTEM] Tap or excite the structure to lock natural resonant frequency.");
    Serial.println("[CONTROLS] Press MYOSA Board BOOT Button to skip pages, or send 'n', 'p', '1'..'3' via Serial\n");
}

void skyMastLoop() {
    uint32_t now = millis();
    g_systemState.lastLoopTime = now;
    g_systemState.uptimeSeconds = now / 1000;

    // Step 1: Manage Calibration to Monitoring State Transition
    if (g_systemState.mode == MODE_CALIBRATING) {
        if (now - g_systemState.calibrationStartMs >= g_systemState.calibrationDurationMs) {
            g_systemState.mode = MODE_MONITORING;
            g_alerts.triggerBuzzerBeeps(2); // Double chirp confirmation
            Serial.println("\n=======================================================");
            Serial.print("[CALIBRATION COMPLETE] Locked Resonant Frequency: ");
            Serial.print(g_systemState.structure.resonantFreqHz, 1);
            Serial.println(" Hz");
            Serial.println("Entering Active Structural Auditing & Safety Interlock Loop...\n");
        }
    }

    // Step 2: Read MYOSA Hardware BOOT Button (GPIO 0)
    static bool lastBtnState = HIGH;
    static uint32_t lastBtnPressTime = 0;
    bool btnState = digitalRead(ONBOARD_BUTTON_PIN);

    if (btnState == LOW && lastBtnState == HIGH && (now - lastBtnPressTime >= 200)) {
        lastBtnPressTime = now;
        g_systemState.currentPage = (g_systemState.currentPage % g_systemState.totalPages) + 1;
        g_alerts.triggerBeep(60);
        Serial.print("[MYOSA BUTTON] Pressed -> Switched to Page ");
        Serial.println(g_systemState.currentPage);
    }
    lastBtnState = btnState;

    // Step 3: Process USB Serial keyboard navigation commands
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == 'n' || c == 'N' || c == ' ') {
            g_systemState.currentPage = (g_systemState.currentPage % g_systemState.totalPages) + 1;
            g_alerts.triggerBeep(60);
            Serial.print("[KEY] Next Page -> Page "); Serial.println(g_systemState.currentPage);
        } else if (c == 'p' || c == 'P') {
            g_systemState.currentPage = (g_systemState.currentPage == 1) ? g_systemState.totalPages : g_systemState.currentPage - 1;
            g_alerts.triggerBeep(60);
            Serial.print("[KEY] Prev Page -> Page "); Serial.println(g_systemState.currentPage);
        } else if (c >= '1' && c <= '3') {
            g_systemState.currentPage = (c - '0');
            g_alerts.triggerBeep(60);
            Serial.print("[KEY] Jump to Page "); Serial.println(g_systemState.currentPage);
        } else if (c == 'a' || c == 'A') {
            g_systemState.autoCyclePages = !g_systemState.autoCyclePages;
            g_alerts.triggerBeep(100);
            Serial.print("[KEY] Auto-Cycle: "); Serial.println(g_systemState.autoCyclePages ? "ENABLED" : "PAUSED/LOCKED");
        }
    }

    // Step 4: High-Speed IMU Read, Tap-Navigation & FFT Vibration Analysis (100 Hz / 10 ms)
    g_imu.update(g_systemState);

    // Step 5: Bernoulli Fluid Dynamics Wind Speed & Atmosphere Update (150 ms)
    g_baro.update(g_systemState);

    // Step 6: Gesture Controller (Non-blocking fallback)
    g_gesture.update(g_systemState);

    // Step 7: Autonomous Safety Interlock & Actuator Evaluation
    g_alerts.update(g_systemState);

    // Step 8: Bluetooth SPP Telemetry Output (100 ms)
    g_bt.update(g_systemState);
    g_systemState.bleConnected = g_bt.isConnected();

    // Step 9: OLED Safety Visuals & Reticle Rendering (80 ms)
    g_display.update(g_systemState);

    // Yield to FreeRTOS / ESP32 tasks
    yield();
}
