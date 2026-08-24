/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: OLED Display Dashboard & Safety Vector Visuals (SSD1306)
 * File: display.h
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <oled.h>
#include "utils.h"

class DisplayController {
public:
    DisplayController();
    bool begin();
    void showStartupScreen();
    void showSensorErrorScreen(const SensorStatus &status);
    bool update(SystemState &state);

private:
    oLed _oled;
    NonBlockingTimer _renderTimer;
    NonBlockingTimer _autoPageTimer;
    uint8_t _animFrame;
    bool _flashToggle;

    void renderHeader(const SystemState &state);
    void renderCalibrationScreen(const SystemState &state);
    void renderPageLevelBubble(const SystemState &state);
    void renderPageWindAndAtmosphere(const SystemState &state);
    void renderPageStructuralResonance(const SystemState &state);
    void renderHazardOverlay(const SystemState &state);
    void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, float value, float minVal, float maxVal);
};

#endif // DISPLAY_H
