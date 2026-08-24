/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: OLED Display Dashboard & Safety Vector Visuals (SSD1306)
 * File: display.cpp
 */

#include "display.h"

DisplayController::DisplayController()
    : _oled(SCREEN_WIDTH, SCREEN_HEIGHT),
      _renderTimer(80),        // 80 ms (~12 FPS) UI render rate
      _autoPageTimer(4000),    // Auto-advance screen every 4.0 seconds
      _animFrame(0),
      _flashToggle(false) {
}

bool DisplayController::begin() {
    bool ok = _oled.begin();
    if (ok) {
        _oled.setFont(NULL);
        _oled.setTextWrap(false);
        _oled.clearDisplay();
        _oled.setTextColor(WHITE, BLACK);
        _oled.display();
    }
    return ok;
}

void DisplayController::showStartupScreen() {
    _oled.clearDisplay();
    _oled.setFont(NULL);

    _oled.drawRoundRect(0, 0, 128, 64, 4, WHITE);
    _oled.drawRoundRect(2, 2, 124, 60, 2, WHITE);

    _oled.setTextSize(2);
    _oled.setTextColor(WHITE, BLACK);
    _oled.setCursor(14, 12);
    _oled.print("SKY-MAST");

    _oled.setTextSize(1);
    _oled.setCursor(12, 34);
    _oled.print("Structural Auditor");

    for (int i = 0; i <= 100; i += 25) {
        _oled.fillRect(14, 48, (i * 100) / 100, 6, WHITE);
        _oled.display();
        delay(50);
    }
    delay(200);
}

void DisplayController::showSensorErrorScreen(const SensorStatus &status) {
    _oled.clearDisplay();
    _oled.setFont(NULL);
    _oled.drawRect(0, 0, 128, 64, WHITE);

    _oled.setTextSize(1);
    _oled.setTextColor(WHITE, BLACK);
    _oled.setCursor(20, 6);
    _oled.print("SENSOR STATUS");

    _oled.drawLine(4, 16, 124, 16, WHITE);

    _oled.setCursor(6, 22);
    _oled.print("IMU :"); _oled.print(status.imuOk ? "OK " : "ERR");
    _oled.setCursor(68, 22);
    _oled.print("BAR :"); _oled.print(status.baroOk ? "OK " : "ERR");

    _oled.setCursor(6, 36);
    _oled.print("ACT :"); _oled.print(status.actuatorOk ? "OK " : "ERR");
    _oled.setCursor(68, 36);
    _oled.print("GEST:BYPASS");

    _oled.setCursor(10, 50);
    _oled.print("Starting audit...");
    _oled.display();
    delay(1000);
}

void DisplayController::drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, float value, float minVal, float maxVal) {
    _oled.drawRect(x, y, w, h, WHITE);
    float norm = (value - minVal) / (maxVal - minVal);
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
    int fillW = (int)(norm * (w - 4));
    if (fillW > 0) {
        _oled.fillRect(x + 2, y + 2, fillW, h - 4, WHITE);
    }
}

void DisplayController::renderHeader(const SystemState &state) {
    _oled.setFont(NULL);
    _oled.fillRect(0, 0, 128, 11, WHITE);
    _oled.setTextColor(BLACK, WHITE);
    _oled.setTextSize(1);

    _oled.setCursor(2, 2);
    _oled.print("SKY-MAST");

    _oled.setCursor(68, 2);
    _oled.print("P");
    _oled.print(state.currentPage);
    _oled.print("/3");

    _animFrame = (_animFrame + 1) % 4;
    char spinChars[] = {'|', '/', '-', '\\'};
    _oled.setCursor(116, 2);
    _oled.print(spinChars[_animFrame]);

    _oled.drawLine(0, 12, 128, 12, WHITE);
    _oled.setTextColor(WHITE, BLACK);
}

void DisplayController::renderCalibrationScreen(const SystemState &state) {
    _oled.setFont(NULL);
    _oled.fillRect(0, 0, 128, 11, WHITE);
    _oled.setTextColor(BLACK, WHITE);
    _oled.setTextSize(1);
    _oled.setCursor(6, 2);
    _oled.print("CALIBRATION MODE");
    _oled.setTextColor(WHITE, BLACK);

    _oled.setCursor(4, 16);
    _oled.print("TAP MAST TO LOCK");

    _oled.setCursor(4, 27);
    _oled.print("NATURAL FREQUENCY");

    _oled.setCursor(4, 39);
    _oled.print("Res Target: ");
    _oled.print(state.structure.resonantFreqHz, 1);
    _oled.print(" Hz");

    uint32_t elapsed = millis() - state.calibrationStartMs;
    float progress = (float)elapsed / (float)state.calibrationDurationMs;
    if (progress > 1.0f) progress = 1.0f;

    _oled.setCursor(4, 52);
    _oled.print("Init: ");
    drawProgressBar(38, 52, 86, 9, progress, 0.0f, 1.0f);
}

void DisplayController::renderPageLevelBubble(const SystemState &state) {
    _oled.setTextSize(1);

    // Left Pane: 2D Level Reticle (Center at (28, 38), Radius 19)
    int16_t cx = 28;
    int16_t cy = 38;
    int16_t r = 19;

    _oled.drawCircle(cx, cy, r, WHITE);
    _oled.drawCircle(cx, cy, 7, WHITE); // Inner safe circle
    _oled.drawLine(cx - r, cy, cx + r, cy, WHITE);
    _oled.drawLine(cx, cy - r, cx, cy + r, WHITE);

    // Calculate 2D bubble position based on Roll (X) and Pitch (Y)
    int16_t bx = cx + (int16_t)(state.orientation.roll * 0.9f);
    int16_t by = cy - (int16_t)(state.orientation.pitch * 0.9f);

    // Clamp bubble to circle boundary
    if (bx < cx - (r - 3)) bx = cx - (r - 3);
    if (bx > cx + (r - 3)) bx = cx + (r - 3);
    if (by < cy - (r - 3)) by = cy - (r - 3);
    if (by > cy + (r - 3)) by = cy + (r - 3);

    _oled.fillCircle(bx, by, 3, WHITE);

    // Right Pane: Deflection & Inclinometer Telemetry
    _oled.setCursor(54, 15);
    _oled.print("Pitch: ");
    if (state.orientation.pitch >= 0) _oled.print("+");
    _oled.print(state.orientation.pitch, 1);
    _oled.print("d");

    _oled.setCursor(54, 27);
    _oled.print("Roll : ");
    if (state.orientation.roll >= 0) _oled.print("+");
    _oled.print(state.orientation.roll, 1);
    _oled.print("d");

    _oled.setCursor(54, 39);
    _oled.print("Defl : ");
    _oled.print(state.orientation.totalDeflection, 1);
    _oled.print("d");

    _oled.setCursor(54, 51);
    _oled.print("MAST : ");
    if (state.highTiltWarning) {
        _oled.print("DANGER");
    } else if (state.orientation.totalDeflection > 8.0f) {
        _oled.print("CAUTION");
    } else {
        _oled.print("STABLE");
    }
}

void DisplayController::renderPageWindAndAtmosphere(const SystemState &state) {
    _oled.setTextSize(1);

    // Line 1: Fluid Dynamics Wind Speed (m/s & km/h)
    _oled.setCursor(2, 15);
    _oled.print("Wind : ");
    _oled.print(state.wind.windSpeedMs, 1);
    _oled.print(" m/s (");
    _oled.print((int)state.wind.windSpeedKmh);
    _oled.print("k)");

    // Line 2: Wind Gauge Progress Bar (0..25 m/s)
    drawProgressBar(2, 26, 124, 7, state.wind.windSpeedMs, 0.0f, 25.0f);

    // Line 3: Dynamic Pressure Drop (Bernoulli dP)
    _oled.setCursor(2, 36);
    _oled.print("dP: ");
    _oled.print(state.wind.dynamicPressurePa, 1);
    _oled.print(" Pa  P:");
    _oled.print(state.env.pressure, 0);
    _oled.print("hPa");

    // Line 4: Environmental Temp & Altitude
    _oled.setCursor(2, 48);
    _oled.print("T:");
    _oled.print(state.env.temperature, 1);
    _oled.print("C Alt:");
    _oled.print((int)state.env.altitude);
    _oled.print("m ");
    if (state.wind.highWindWarning) _oled.print("GALE");
    else if (state.wind.windSpeedMs > 10.0f) _oled.print("GUST");
    else _oled.print("NORM");
}

void DisplayController::renderPageStructuralResonance(const SystemState &state) {
    _oled.setTextSize(1);

    // Line 1: Locked Resonant Frequency
    _oled.setCursor(2, 15);
    _oled.print("Res Locked: ");
    _oled.print(state.structure.resonantFreqHz, 1);
    _oled.print(" Hz");

    // Line 2: Live Dominant Sway Frequency
    _oled.setCursor(2, 27);
    _oled.print("Live Peak : ");
    if (state.structure.livePeakFreqHz > 0.0f) {
        _oled.print(state.structure.livePeakFreqHz, 1);
        _oled.print(" Hz");
    } else {
        _oled.print("0.0 Hz (IDLE)");
    }

    // Line 3: Resonant Energy Level Bar (0..8)
    _oled.setCursor(2, 39);
    _oled.print("Res E:");
    drawProgressBar(40, 39, 86, 8, state.structure.resonantBandEnergy, 0.0f, 8.0f);

    // Line 4: Sustained Cycles & Safety Status
    _oled.setCursor(2, 51);
    _oled.print("Sway: ");
    if (state.structure.resonanceHazard) {
        _oled.print("RESONANCE TRIP");
    } else if (state.structure.sustainedCycles > 0) {
        _oled.print("RESONATING...");
    } else {
        _oled.print("SAFE (NO HAZ)");
    }
}

void DisplayController::renderHazardOverlay(const SystemState &state) {
    _flashToggle = !_flashToggle;

    _oled.fillRect(0, 13, 128, 51, BLACK);

    if (_flashToggle) {
        _oled.fillRect(2, 14, 124, 48, WHITE);
        _oled.setTextColor(BLACK, WHITE);
    } else {
        _oled.drawRect(2, 14, 124, 48, WHITE);
        _oled.drawRect(4, 16, 120, 44, WHITE);
        _oled.setTextColor(WHITE, BLACK);
    }

    _oled.setTextSize(1);
    _oled.setCursor(10, 18);
    _oled.print("!!! CRITICAL ALERT !!!");

    _oled.setCursor(8, 30);
    if (state.structure.resonanceHazard) {
        _oled.print("STRUCTURAL RESONANCE");
        _oled.setCursor(12, 40);
        _oled.print("Freq: ");
        _oled.print(state.structure.livePeakFreqHz, 1);
        _oled.print(" Hz MATCH");
    } else if (state.wind.highWindWarning) {
        _oled.print("GALE WIND EXCEEDED");
        _oled.setCursor(14, 40);
        _oled.print("Wind: ");
        _oled.print(state.wind.windSpeedMs, 1);
        _oled.print(" m/s");
    } else {
        _oled.print("CRITICAL DEFLECTION");
        _oled.setCursor(14, 40);
        _oled.print("Defl: ");
        _oled.print(state.orientation.totalDeflection, 1);
        _oled.print(" deg");
    }

    _oled.setCursor(8, 50);
    _oled.print("CRANE INTERLOCK OPEN");

    _oled.setTextColor(WHITE, BLACK);
}

bool DisplayController::update(SystemState &state) {
    // Auto-advance screen view in monitoring mode (every 4.0 seconds) if enabled
    if (state.mode == MODE_MONITORING && state.autoCyclePages && _autoPageTimer.isReady()) {
        state.currentPage = (state.currentPage % state.totalPages) + 1;
    }

    if (!_renderTimer.isReady()) {
        return false;
    }

    _oled.clearDisplay();

    // Check if in startup calibration mode
    if (state.mode == MODE_CALIBRATING) {
        renderCalibrationScreen(state);
    } else {
        // Render standard header
        renderHeader(state);

        // If any hazard is active, show emergency interlock overlay
        if (state.structure.resonanceHazard || state.wind.highWindWarning || state.highTiltWarning) {
            renderHazardOverlay(state);
        } else {
            // Render active dashboard view
            switch (state.currentPage) {
                case 1:
                    renderPageLevelBubble(state);
                    break;
                case 2:
                    renderPageWindAndAtmosphere(state);
                    break;
                case 3:
                    renderPageStructuralResonance(state);
                    break;
                default:
                    renderPageLevelBubble(state);
                    break;
            }
        }
    }

    _oled.display();
    return true;
}
