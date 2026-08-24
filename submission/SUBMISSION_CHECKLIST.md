# IEEE MYOSA Project Submission Checklist

This document tracks the technical preparation, media asset generation, and final submission steps for **Sky-Mast: Autonomous Smart Crane & Structure Safety Auditor**.

---

## ✅ AUTOMATED / ALREADY COMPLETED IN CODEBASE

The following firmware modules, algorithms, tests, and documentation have been fully built, verified, and flashed to the hardware:

* [x] **64-Point Radix-2 FFT Engine**: Embedded C++ implementation running at 100 Hz ($0\text{ to }50\text{ Hz}$ span, $\Delta f = 1.56\,\text{Hz}$) with Hanning windowing and parabolic sub-bin peak interpolation ([`fft_analyzer.cpp`](file:///home/sreyas/myosa-accel-demo/fft_analyzer.cpp)).
* [x] **Bernoulli Fluid Dynamics Wind Model**: Dynamic stagnation pressure drop conversion ($v = \sqrt{2\Delta P / \rho}$) with adaptive baseline tracking and noise gating in [`barometer.cpp`](file:///home/sreyas/myosa-accel-demo/barometer.cpp).
* [x] **Dual-Stage Structural Resonance Interlock**: 5.0-second startup Tap-Test calibration locking $f_{\text{res}}$ baseline, plus continuous 5-cycle sustained resonance latch in [`imu.cpp`](file:///home/sreyas/myosa-accel-demo/imu.cpp).
* [x] **Multi-View 128x64 OLED Interface**: 2D Level Reticle & floating bubble, Bernoulli wind dial, FFT resonance meter, and full-screen hazard overlay in [`display.cpp`](file:///home/sreyas/myosa-accel-demo/display.cpp).
* [x] **Manual Hardware Button Navigation**: Onboard tactile `BOOT` button (GPIO 0) configured with 200 ms debounced page skipping in [`main.cpp`](file:///home/sreyas/myosa-accel-demo/main.cpp).
* [x] **Autonomous Actuator Safety Shutdown**: PCA9536 Relay power cutoff (`AC_SWITCH_IO` $\rightarrow$ LOW) and multi-pin siren pulsing on GPIOs D4, D12, D13, D27 in [`alerts.cpp`](file:///home/sreyas/myosa-accel-demo/alerts.cpp).
* [x] **IEEE 33-Value Telemetry Engine**: Continuous Bluetooth Classic SPP telemetry stream (`MYOSA_1`) at 115200 baud in [`bt.cpp`](file:///home/sreyas/myosa-accel-demo/bt.cpp).
* [x] **Clean Git Configuration**: Created [`.gitignore`](file:///home/sreyas/myosa-accel-demo/.gitignore) excluding build caches, binaries, and temporary logs; zero credentials or secrets exposed.
* [x] **Submission Directory & Documentation**: Generated [`submission/blog.md`](file:///home/sreyas/myosa-accel-demo/submission/blog.md) and [`submission/README.md`](file:///home/sreyas/myosa-accel-demo/submission/README.md) with structured technical sections and placeholders.
* [x] **Hardware Build & Flash Verification**: Compiled and flashed via `arduino-cli` with 0 errors / 0 warnings on ESP32 target.

---

## 📝 MANUAL ACTION REQUIRED BY AUTHOR

Before submitting to the competition portal or Google Form, you must complete the following items in order:

---

### 1. Capture Hardware Photos & OLED Screenshots
* **What to do**: Take clear photos of your MYOSA hardware and the OLED screen displaying each of the 3 pages.
* **Where to put them**: Save files into `submission/assets/images/`:
  - `project_hero.png` (Overall hardware setup / mast rig photo)
  - `architecture_diagram.png` (Block diagram of hardware and data flow)
  - `screenshot_page1.png` (OLED displaying Page 1: 2D Level Bubble)
  - `screenshot_page2.png` (OLED displaying Page 2: Bernoulli Wind Speed)
  - `screenshot_page3.png` (OLED displaying Page 3: FFT Structural Resonance)
  - `video_thumbnail.png` (Thumbnail graphic for your demo video)
* **Verification**: Open `submission/assets/images/` and verify that all image files open properly and are not corrupted.

---

### 2. Record & Upload the Project Demonstration Video
* **What to do**: Record a 2 to 3-minute video showing:
  1. Booting the board and performing the startup **Tap-Test calibration** to lock baseline frequency.
  2. Tilting the board to show the **2D Level Bubble** floating inside the reticle on Page 1.
  3. Blowing or using a fan across the enclosure to show live **Bernoulli Wind Speed** calculation on Page 2.
  4. Oscillating/vibrating the mast at the locked resonant frequency on Page 3 to trigger the **autonomous relay cutoff, alarm siren, and critical screen overlay**.
* **Where to host**: Upload the video to **YouTube** (Public / Unlisted) or **Google Drive** (Set link permissions to "Anyone with the link can view").
* **What to produce**: A shareable video URL.
* **Verification**: Test the video link in an incognito/private browser tab to confirm anyone can watch it without login issues.

---

### 3. Create a Public GitHub Repository & Push the Codebase
* **What to do**:
  1. Open [GitHub](https://github.com/new) and create a new repository (e.g. `sky-mast-myosa`).
  2. Ensure the repository visibility is set to **PUBLIC**.
  3. Initialize and push your project:
     ```bash
     cd /home/sreyas/myosa-accel-demo
     git init
     git add .
     git commit -m "Initial commit: Sky-Mast IEEE MYOSA Technical Submission"
     git branch -M main
     git remote add origin https://github.com/<YOUR_USERNAME>/<REPO_NAME>.git
     git push -u origin main
     ```
* **What to produce**: Public GitHub repository URL.
* **Verification**: Open your GitHub repository URL in an incognito browser window to verify all code, `submission/blog.md`, and assets are visible publicly.

---

### 4. Fill in All Placeholders in `submission/blog.md`
* **What to do**: Open `submission/blog.md` and replace the following placeholder tags with your actual details:
  - `[INSERT PROJECT HERO IMAGE]` $\rightarrow$ Link to `assets/images/project_hero.png`
  - `[INSERT ARCHITECTURE DIAGRAM]` $\rightarrow$ Link to `assets/images/architecture_diagram.png`
  - `[INSERT SCREENSHOT - 2D LEVEL BUBBLE]` $\rightarrow$ Link to `assets/images/screenshot_page1.png`
  - `[INSERT SCREENSHOT - BERNOULLI WIND GAUGE]` $\rightarrow$ Link to `assets/images/screenshot_page2.png`
  - `[INSERT SCREENSHOT - FFT STRUCTURAL RESONANCE]` $\rightarrow$ Link to `assets/images/screenshot_page3.png`
  - `[INSERT DEMO VIDEO LINK]` $\rightarrow$ Your YouTube / Drive video link
  - `[INSERT GITHUB REPOSITORY LINK]` $\rightarrow$ Your public GitHub repository link
  - `[INSERT AUTHOR/TEAM INFORMATION]` $\rightarrow$ Team member names, department, institution, and contact email
* **Verification**: Search `submission/blog.md` for `[` or `INSERT` to ensure no unreplaced bracket tags remain.

---

### 5. Final Submission Form Entry
* **What to do**: Open the competition submission Google Form / portal provided by the organizers and submit:
  1. Project Title: **Sky-Mast: Autonomous Smart Crane & Structure Safety Auditor**
  2. Public GitHub Repository Link
  3. Direct Link to `submission/blog.md` on GitHub
  4. Project Demonstration Video Link
  5. Team & Contact Information
* **Verification**: Review all form entries before clicking "Submit". Save confirmation receipt or email.
