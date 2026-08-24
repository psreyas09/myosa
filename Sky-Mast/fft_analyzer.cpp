/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Fast Fourier Transform (FFT) Structural Vibration Analyzer
 * File: fft_analyzer.cpp
 */

#include "fft_analyzer.h"

StructuralFFTAnalyzer::StructuralFFTAnalyzer()
    : _sampleIndex(0), _sampleCount(0) {
    // Pre-calculate Hanning window coefficients to minimize runtime CPU cycles
    for (int i = 0; i < FFT_N; i++) {
        _window[i] = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * (float)i / (float)(FFT_N - 1)));
        _sampleBuffer[i] = 0.0f;
        _workReal[i] = 0.0f;
        _workImag[i] = 0.0f;
    }
}

void StructuralFFTAnalyzer::reset() {
    _sampleIndex = 0;
    _sampleCount = 0;
    for (int i = 0; i < FFT_N; i++) {
        _sampleBuffer[i] = 0.0f;
    }
}

void StructuralFFTAnalyzer::addSample(float sample) {
    _sampleBuffer[_sampleIndex] = sample;
    _sampleIndex = (_sampleIndex + 1) % FFT_N;
    if (_sampleCount < FFT_N) {
        _sampleCount++;
    }
}

bool StructuralFFTAnalyzer::isReady() const {
    return _sampleCount >= FFT_N;
}

void StructuralFFTAnalyzer::radix2FFT() {
    // 1. Bit-reversal permutation on _workReal and _workImag
    int j = 0;
    for (int i = 0; i < FFT_N - 1; i++) {
        if (i < j) {
            float tempR = _workReal[i];
            float tempI = _workImag[i];
            _workReal[i] = _workReal[j];
            _workImag[i] = _workImag[j];
            _workReal[j] = tempR;
            _workImag[j] = tempI;
        }
        int k = FFT_N / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }

    // 2. Cooley-Tukey Radix-2 Butterfly computation
    for (int len = 2; len <= FFT_N; len <<= 1) {
        float angle = -2.0f * (float)M_PI / (float)len;
        float wlenR = cosf(angle);
        float wlenI = sinf(angle);

        for (int i = 0; i < FFT_N; i += len) {
            float wR = 1.0f;
            float wI = 0.0f;
            for (int k = 0; k < len / 2; k++) {
                int u = i + k;
                int v = i + k + len / 2;

                float vR = _workReal[v] * wR - _workImag[v] * wI;
                float vI = _workReal[v] * wI + _workImag[v] * wR;

                _workReal[v] = _workReal[u] - vR;
                _workImag[v] = _workImag[u] - vI;
                _workReal[u] = _workReal[u] + vR;
                _workImag[u] = _workImag[u] + vI;

                float nextWR = wR * wlenR - wI * wlenI;
                float nextWI = wR * wlenI + wI * wlenR;
                wR = nextWR;
                wI = nextWI;
            }
        }
    }
}

void StructuralFFTAnalyzer::compute(float &peakFreqHz, float &peakMagnitude, float &resonantBandEnergy, float centerResonantFreqHz, float bandWidthHz) {
    if (_sampleCount < FFT_N) {
        peakFreqHz = 0.0f;
        peakMagnitude = 0.0f;
        resonantBandEnergy = 0.0f;
        return;
    }

    // 1. Copy circular sample buffer in chronological order into linear working buffer
    float mean = 0.0f;
    for (int i = 0; i < FFT_N; i++) {
        int idx = (_sampleIndex + i) % FFT_N;
        _workReal[i] = _sampleBuffer[idx];
        mean += _workReal[i];
    }
    mean /= (float)FFT_N;

    // 2. Subtract DC bias and apply Hanning window
    for (int i = 0; i < FFT_N; i++) {
        _workReal[i] = (_workReal[i] - mean) * _window[i];
        _workImag[i] = 0.0f;
    }

    // 3. Run in-place Cooley-Tukey Radix-2 FFT on working copy
    radix2FFT();

    // 4. Compute magnitude spectrum for bins k = 1 .. N/2 (1.56 Hz to 50 Hz)
    float binWidth = (float)FFT_SAMPLE_RATE / (float)FFT_N; // 100 / 64 = 1.5625 Hz
    float maxMag = 0.0f;
    int maxBin = 1;

    float mag[FFT_N / 2];
    mag[0] = 0.0f; // DC 0 Hz ignored

    for (int k = 1; k < FFT_N / 2; k++) {
        float m = sqrtf(_workReal[k] * _workReal[k] + _workImag[k] * _workImag[k]);
        mag[k] = m;
        if (m > maxMag) {
            maxMag = m;
            maxBin = k;
        }
    }

    peakMagnitude = maxMag;

    // Robust Noise Gate: Baseline ADC noise floor is ~0.08 - 0.12.
    // Threshold set to 0.20 to completely eliminate false noise peaks when at rest.
    if (maxMag < 0.20f) {
        peakFreqHz = 0.0f;
        resonantBandEnergy = 0.0f;
        return;
    }

    // Parabolic interpolation around peak bin for sub-bin precision frequency
    if (maxBin > 1 && maxBin < (FFT_N / 2 - 1)) {
        float alpha = mag[maxBin - 1];
        float beta  = mag[maxBin];
        float gamma = mag[maxBin + 1];
        float denom = 2.0f * (2.0f * beta - alpha - gamma);
        float delta = (denom != 0.0f) ? (gamma - alpha) / denom : 0.0f;
        peakFreqHz = ((float)maxBin + delta) * binWidth;
    } else {
        peakFreqHz = (float)maxBin * binWidth;
    }

    // Integrate energy in resonant band [centerResonantFreqHz - bandWidthHz, centerResonantFreqHz + bandWidthHz]
    float totalBandEnergy = 0.0f;
    for (int k = 1; k < FFT_N / 2; k++) {
        float freq = (float)k * binWidth;
        if (fabsf(freq - centerResonantFreqHz) <= bandWidthHz) {
            totalBandEnergy += (mag[k] * mag[k]);
        }
    }
    resonantBandEnergy = sqrtf(totalBandEnergy);
}
