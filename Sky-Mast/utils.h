/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Utilities & Data Definitions
 * File: utils.h
 */

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <math.h>

// Operational System Modes
enum SystemMode {
    MODE_CALIBRATING,  // Startup Tap-Test mode to identify mast natural resonant frequency
    MODE_MONITORING,   // Active real-time structural auditing & safety monitoring
    MODE_SHUTDOWN      // Safety interlock triggered / Crane operations locked
};

// Sensor Connection Status Structure
struct SensorStatus {
    bool imuOk = false;
    bool baroOk = false;
    bool gestureOk = false;
    bool actuatorOk = false;
};

// Environmental Sensor Measurements
struct EnvironmentalData {
    float altitude = 0.0f;       // Altitude in meters (m)
    float pressure = 0.0f;       // Air pressure in Hectopascals (hPa)
    float temperature = 0.0f;    // Temperature in Degrees Celsius (°C)
    float prevAltitude = 0.0f;   // Previous altitude reading for rate measurement
    float altitudeRate = 0.0f;   // Altitude change rate in m/s
};

// Structural Fluid Dynamics Wind Data (Bernoulli Pressure Drop Model)
struct WindData {
    float windSpeedMs = 0.0f;        // Estimated wind speed in m/s
    float windSpeedKmh = 0.0f;       // Wind speed in km/h
    float dynamicPressurePa = 0.0f;  // Differential Bernoulli stagnation pressure drop (Pa)
    bool highWindWarning = false;    // Wind speed > 17.0 m/s (~60 km/h)
};

// Orientation Measurements
struct OrientationData {
    float pitch = 0.0f;          // Pitch angle (-180° to 180°)
    float roll = 0.0f;           // Roll angle (-180° to 180°)
    float tiltX = 0.0f;          // Tilt X axis angle (degrees)
    float tiltY = 0.0f;          // Tilt Y axis angle (degrees)
    float tiltZ = 0.0f;          // Tilt Z axis angle (degrees)
    float totalDeflection = 0.0f;// Total 2D angular deflection vector (sqrt(Pitch^2 + Roll^2))
};

// Motion Data Measurements
struct MotionData {
    float accelX = 0.0f;         // Acceleration X (g)
    float accelY = 0.0f;         // Acceleration Y (g)
    float accelZ = 0.0f;         // Acceleration Z (g)
    float gyroX = 0.0f;          // Gyroscope X (°/s)
    float gyroY = 0.0f;          // Gyroscope Y (°/s)
    float gyroZ = 0.0f;          // Gyroscope Z (°/s)
    bool motionDetected = false; // MPU6050 motion detection status
};

// Structural Frequency & FFT Resonance Analysis Data
struct StructuralFrequencyData {
    float resonantFreqHz = 12.5f;    // Natural resonant frequency locked from Tap-Test
    float livePeakFreqHz = 0.0f;     // Real-time dominant vibration frequency from FFT
    float peakMagnitude = 0.0f;      // Real-time peak FFT magnitude
    float resonantBandEnergy = 0.0f; // Energy integrated within resonant band (f_res +/- 2.5 Hz)
    bool tapDetected = false;        // Tap-test excitation captured during calibration
    uint8_t sustainedCycles = 0;     // Sustained vibration cycles near resonant band
    bool resonanceHazard = false;    // Critical structural resonance condition
};

// Global Application State Container
struct SystemState {
    SystemMode mode = MODE_CALIBRATING;
    uint32_t calibrationStartMs = 0;
    uint32_t calibrationDurationMs = 5000; // 5.0 second tap-test window

    SensorStatus sensorStatus;
    EnvironmentalData env;
    WindData wind;
    OrientationData orientation;
    MotionData motion;
    StructuralFrequencyData structure;

    float proxValue = 0.0f;
    char lastGestureStr[16] = "";

    uint8_t currentPage = 1;      // Active page index (1..3)
    const uint8_t totalPages = 3; // Total active pages (Level Bubble, Wind/Env, FFT Resonance)
    bool autoCyclePages = false;  // Auto-cycling disabled by default (use MYOSA board button to switch)

    bool highTiltWarning = false;    // Severe mast tilt > 45° or deflection > 15°
    bool rapidAltWarning = false;    // Altitude changing rapidly (>1.5 m/s)
    bool mastInterlockEngaged = true;// Crane power active (true) vs Safety lockout cut (false)

    bool bleActive = true;        // BLE status flag
    bool bleConnected = false;     // BLE connection status flag
    bool wifiActive = false;      // Wi-Fi status flag
    uint32_t lastLoopTime = 0;    // Timestamp of last execution loop
    uint32_t uptimeSeconds = 0;   // System uptime in seconds
};

// Non-blocking Timer Class using millis()
class NonBlockingTimer {
public:
    NonBlockingTimer(uint32_t intervalMs = 100);
    void setInterval(uint32_t intervalMs);
    bool isReady();
    void reset();

private:
    uint32_t _interval;
    uint32_t _previousMillis;
};

// Utility Math & String Helpers
float calculatePitch(float accelX, float accelY, float accelZ);
float calculateRoll(float accelX, float accelY, float accelZ);
void formatFloat(float value, char* buffer, size_t bufferLen, uint8_t decimals = 1);

#endif // UTILS_H
