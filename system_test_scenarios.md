# ASIC Mobile Robot — System Test Scenarios (Core Features)

This document provides comprehensive test cases for validating all non-chatbot subsystems of the mobile robot. It spans UI flows, state machine transitions, C++ ViewModels, hardware integration bounds, and telemetry logging outputs.

---

## Telemetry Log Reference
All actions in these test cases trigger specific entries in your JSON log files (`~/.local/share/frontend_app/logs/session_*.json`).
- Look for `cat: "boot"`, `cat: "navigation"`, `cat: "ui"`, `cat: "ui_latency"`, and `cat: "perf"`.

---

## 1. System Boot & Pre-Check (SysCheckView)

Tests the startup check automation, script triggering, and initial logging file lifecycle.

| ID | Test Scenario | Steps | Expected UI / Hardware Result | Telemetry Output |
|---|---|---|---|---|
| **BC1** | Cold Boot Sequence (Ready) | 1. Launch the application.<br>2. Observe the Pre-Check screen. | - Progress bar completes automatically.<br>- No failures detected.<br>- App automatically navigates to `MainView` after 2s. | - Log file created with header.<br>- `cat: "boot"`, `event: "syscheck_complete"` (pass count, 0 failures). |
| **BC2** | Boot Pre-Check Failure | 1. Simulate a missing sensor or script failure.<br>2. Restart the app. | - Progress bar halts.<br>- Failed systems show a red indicator.<br>- App remains locked on the Pre-Check screen. | - `syscheck_complete` logged with `fail` count > 0.<br>- No navigation start. |
| **BC3** | Manual Nav Launch / Relaunch | 1. In settings, stop navigation.<br>2. Re-trigger "Launch Navigation". | - `run_nav.sh` script executes.<br>- State transitions to running. | - `event: "nav_process_started"`.<br>- Previous session closes and new session file is generated. |

---

## 2. Direction View & Destination Selection (DirectionView)

Tests map interactions, room/checkpoint selection, floor switches, and confirm prompts.

| ID | Test Scenario | Steps | Expected UI / Hardware Result | Telemetry Output |
|---|---|---|---|---|
| **DV1** | Floor Switching | 1. Tap the floor switch icon.<br>2. Select a different floor (e.g., F1 → F2). | - Map background updates to new floor map.<br>- Checkpoints list updates for the new floor. | - `cat: "ui"`, `event: "state_transition"` (if view restarts). |
| **DV2** | Room Navigation Request | 1. Tap a checkpoint or room name on the screen.<br>2. Tap "Yes" in the confirmation popup. | - Confirmation popup disappears.<br>- App shifts to `RunningView`. | - `cat: "ui_latency"`, `action: "navigate_to_cp"` recorded.<br>- `cat: "navigation"`, `event: "navigate_command"` with target cpId. |
| **DV3** | Confirmation Cancel | 1. Tap a checkpoint.<br>2. Tap "No" / Cancel in the popup. | - Popup closes.<br>- App remains on `DirectionView`. | - No navigation command sent. |
| **DV4** | Auto-Home Timeout Timer | 1. Stay idle on `DirectionView` for 15s.<br>2. Watch the home countdown timer. | - Confirmation popup for home navigation displays.<br>- Timer counts down and triggers home navigation. | - `action: "navigate_to_cp"` with `cpId: 0` (Home). |

---

## 3. Active Running View (RunningView)

Tests control mechanisms while the robot is in motion (state changes, resets, emergency overrides).

| ID | Test Scenario | Steps | Expected UI / Hardware Result | Telemetry Output |
|---|---|---|---|---|
| **RV1** | Live Progress Updates | 1. Trigger navigation to room.<br>2. Observe active status cards. | - UI shows current moving speed and destination.<br>- Status message updates. | - `cat: "navigation"`, `event: "status"` updates.<br>- `cat: "navigation"`, `event: "state_changed"`. |
| **RV2** | Emergency Stop | 1. While robot is moving, tap the big red "Stop" button. | - Robot immediately halts physical movement.<br>- Stop button changes to a green "Continue" button.<br>- Background colors alert user. | - `cat: "ui_latency"`, `action: "emergency_stop"`.<br>- `cat: "navigation"`, `event: "emergency_stop"` (`active: true`). |
| **RV3** | Emergency Resume | 1. Tap the green "Continue" button after an emergency stop. | - Robot resumes moving toward its original target.<br>- Continue button toggles back to red "Stop" button. | - `cat: "ui_latency"`, `action: "emergency_resume"`.<br>- `cat: "navigation"`, `event: "emergency_stop"` (`active: false`). |
| **RV4** | Reset Mid-Navigation | 1. While moving, tap the "Reset" button. | - Robot halts and aborts task.<br>- App transitions back to `DirectionView`. | - `cat: "ui_latency"`, `action: "reset_direction"`.<br>- `cat: "navigation"`, `event: "reset_requested"`. |
| **RV5** | Arrival at Checkpoint | 1. Let the robot reach the targeted checkpoint. | - App shows "Arrived at destination".<br>- 15-second countdown timer starts.<br>- App returns to `DirectionView` when timer hits zero. | - `cat: "navigation"`, `event: "checkpoint_arrived"`. |

---

## 4. Presentation & QR Code Upload (PresentationView)

Tests file management, mobile upload bridging, and presentation slideshow rendering.

| ID | Test Scenario | Steps | Expected UI / Hardware Result | Telemetry Output |
|---|---|---|---|---|
| **PV1** | QR Code Generation | 1. Open the Presentation View. | - Screen displays a clear QR code containing local IP address upload link. | - `cat: "ui"`, `event: "state_transition"`. |
| **PV2** | Web Server Upload (.txt) | 1. Scan QR from mobile.<br>2. Upload a text file with notes. | - Mobile page shows success.<br>- Robot screen displays uploaded text content in document viewer. | - Warnings logged to `cat: "error"` if upload fails. |
| **PV3** | Web Server Upload (.pptx) | 1. Upload a PowerPoint presentation file (.pptx) via QR link. | - File converts / loads.<br>- Slide viewer becomes active with first slide. | - Log file captures system metrics change. |
| **PV4** | Presentation Controls | 1. Tap Next / Previous slide controls. | - Carousel moves forward / backward.<br>- Slide indicator updates (e.g., 2/10). | - UI responsiveness latency log check. |

---

## 5. Control Center & Diagnostics (ControlCenterView)

Tests security gates, performance logging fidelity, and sensor diagnostic displays.

| ID | Test Scenario | Steps | Expected UI / Hardware Result | Telemetry Output |
|---|---|---|---|---|
| **CC1** | Admin Gate Protection | 1. Ensure role is `User`.<br>2. Tap "Control Center". | - Access denied prompt / login request displays.<br>- Prevent transition to diagnostics. | - Unauthorized access attempt warning in Qt logs (captured by `SessionLogger` msg handler). |
| **CC2** | Live Performance Polling | 1. Navigate to Diagnostics View (requires Admin/Developer). | - Interactive line graphs of CPU, RAM, Network, and FPS load update every second. | - `cat: "perf"` records added to JSON log at 1Hz showing exact matching graph values. |
| **CC3** | Low FPS Alert Validation | 1. Simulate low system performance / CPU stress. | - Diagnostics page updates.<br>- App flags warning if FPS drops below 15. | - `cat: "perf"`, `fps` value recorded reflects drops.<br>- Warning caught in `cat: "error"`. |

---

## 6. Access Control & Wifi Settings (SettingsView)

Tests credentials verification, wifi hookup, and session inactivity timeout thresholds.

| ID | Test Scenario | Steps | Expected UI / Hardware Result | Telemetry Output |
|---|---|---|---|---|
| **US1** | Admin Role Elevation | 1. Settings → Switch Mode.<br>2. Select Admin, enter password. | - Screen returns to MainView with "Admin" badge visible in container bar. | - `cat: "ui"`, `event: "state_transition"` settings -> login. |
| **US2** | Idle Logout Timeout | 1. Log in as Admin.<br>2. Do not touch screen for 3 minutes. | - Application automatically transitions to User mode.<br>- "Admin" badge disappears, replaced by "User". | - Inactivity reversion logs. |
| **US3** | Wi-Fi Scan & Connection | 1. Go to Settings → Wi-Fi.<br>2. Tap scan, select a network, enter password. | - Network list populates.<br>- Selected network shows connecting spinner, then connects. | - If connection fails, error output is caught by message handler log. |

---

## Recommended Complete System Test Sequence

To test all components while logging a clean telemetry file:

1. **Start System:** Power on, wait for `SysCheckView` to successfully pass (starts telemetry recording).
2. **Settings check:** Elevate role to `Admin`, inspect wifi settings.
3. **Diagnostics check:** Open `Control Center` → `Diagnostics` and watch graphs populate for 10 seconds.
4. **Trigger Nav:** Return to `Direction View`, select a Room, and confirm.
5. **Run tests:** During navigation, test `Emergency Stop`, wait 3s, then `Continue`.
6. **Arrive:** Let the robot arrive at the destination, wait for the auto-return timer to bring it home.
7. **Presentation:** Open `Presentation View`, upload a test file, and scroll through 3 slides.
8. **End:** Stop the application. Retrieve the output `.json` file from XDG data path and run the validator script.
