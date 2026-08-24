/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Fast Fourier Transform (FFT) Structural Vibration Analyzer
 * File: fft_analyzer.h
 */

#ifndef FFT_ANALYZER_H
#define FFT_ANALYZER_H

#include <Arduino.h>
#include <math.h>

#define FFT_N 64          // 64-point FFT
#define FFT_SAMPLE_RATE 100 // 100 Hz sampling rate (10 ms period, Nyquist = 50 Hz)

class StructuralFFTAnalyzer {
public:
    StructuralFFTAnalyzer();
    
    // Reset buffer
    void reset();

    // Push new sample from accelerometer into circular sample buffer
    void addSample(float sample);

    // Check if 64-sample buffer is filled and ready for FFT
    bool isReady() const;

    // Run In-Place Radix-2 FFT on a snapshot of the buffer without corrupting sample history
    void compute(float &peakFreqHz, float &peakMagnitude, float &resonantBandEnergy, float centerResonantFreqHz, float bandWidthHz = 2.5f);

private:
    float _sampleBuffer[FFT_N]; // Dedicated circular time-domain sample buffer
    float _workReal[FFT_N];     // Working array for FFT calculation
    float _workImag[FFT_N];     // Working array for FFT calculation
    float _window[FFT_N];       // Pre-calculated Hanning window coefficients
    uint16_t _sampleIndex;
    uint16_t _sampleCount;

    void radix2FFT();
};

#endif // FFT_ANALYZER_H
