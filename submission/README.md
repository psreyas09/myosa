# Sky-Mast Submission Assets & Guide

This directory contains the formal documentation, media assets, and verification checklist for the **IEEE MYOSA Technical Project Submission**.

---

## 📁 Directory Structure

```
submission/
├── blog.md                    # Main technical submission blog (formatted in Markdown)
├── README.md                  # This submission guide
├── SUBMISSION_CHECKLIST.md    # Actionable verification checklist (Automated vs Manual tasks)
└── assets/
    ├── images/                # Supporting photos, screenshots, and diagrams
    │   ├── project_hero.png       <- Place high-res project photo / hardware banner here
    │   ├── architecture_diagram.png <- Place hardware & software block diagram here
    │   ├── screenshot_page1.png   <- Place OLED Page 1 (2D Level Bubble) photo here
    │   ├── screenshot_page2.png   <- Place OLED Page 2 (Wind Speed Gauge) photo here
    │   ├── screenshot_page3.png   <- Place OLED Page 3 (FFT Resonance Monitor) photo here
    │   └── video_thumbnail.png    <- Place YouTube / video thumbnail image here
    └── videos/                # Local demo video clips / presentation slides
        └── demo_recording.mp4     <- Local copy of the project demonstration video
```

---

## 📸 Where to Place Images & Media

1. **Hardware Photos & OLED Screen Captures**:
   - Take crisp photos or screenshots of the physical OLED screen running each of the 3 pages.
   - Save them into `submission/assets/images/` using the exact file names referenced above.
2. **Architecture Diagram**:
   - Save your hardware block diagram or system flow image as `submission/assets/images/architecture_diagram.png`.

---

## 🎥 Where to Add the Demonstration Video Link

1. Record a **2 to 3-minute video** demonstrating:
   - Startup Tap-Test calibration (locking baseline resonant frequency).
   - Live 2D Level Bubble inclinometer response.
   - Bernoulli wind speed estimation (blowing/fan across the enclosure).
   - Resonant vibration matching triggering the autonomous relay cutoff and alarm.
2. Upload the video to **YouTube** (Unlisted or Public) or **Google Drive** (access set to "Anyone with the link can view").
3. Open `submission/blog.md` and replace the placeholder `[INSERT DEMO VIDEO LINK]` with your actual URL.

---

## ✍️ Information to Fill in Manually in `submission/blog.md`

Before final submission, search `submission/blog.md` and fill in:
* `[INSERT GITHUB REPOSITORY LINK]` $\rightarrow$ Your public GitHub repository URL (e.g. `https://github.com/username/myosa-sky-mast`)
* `[INSERT AUTHOR/TEAM INFORMATION]` $\rightarrow$ Your name(s), institution, department, and contact email.
* `[INSERT DEMO VIDEO LINK]` $\rightarrow$ Your YouTube/Drive demo video link.

---

## 🔍 Pre-Submission Verification Steps

1. **Test Markdown Rendering**:
   - Open `submission/blog.md` in a Markdown viewer or on GitHub to ensure all tables, code blocks, and diagrams render cleanly.
2. **Verify Image Links**:
   - Ensure all image links in `blog.md` resolve correctly relative to the repository.
3. **Verify Public GitHub Access**:
   - Open your repository in a private/incognito browser tab to confirm it is publicly accessible without login.
