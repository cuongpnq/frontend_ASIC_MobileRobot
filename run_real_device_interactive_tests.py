#!/usr/bin/env python3
"""
ASIC Mobile Robot — Real Device Interactive Test Runner
Covers hardware/UI system test scenarios: BC, DV, RV, PV, CC, US.

Chatbot tests are in a separate script:
  python3 run_chatbot_tests.py

Run with:
  python3 run_real_device_interactive_tests.py
"""
import os
import sys
import json
import time

LOG_DIR = os.path.expanduser("~/.local/share/frontend_app/logs")

GREEN  = "\033[92m"
YELLOW = "\033[93m"
RED    = "\033[91m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
RESET  = "\033[0m"

# ── helpers ────────────────────────────────────────────────────────────────────

def hdr(text):
    print(f"\n{BOLD}{CYAN}# {'=' * 74}\n# {text}\n# {'=' * 74}{RESET}")

def ok(msg):  print(f"  {GREEN}[PASS]{RESET} {msg}")
def warn(msg):print(f"  {YELLOW}[WARN]{RESET} {msg}")
def err(msg): print(f"  {RED}[FAIL]{RESET} {msg}")

def ask(prompt):
    """Ask tester a yes/no question and return bool."""
    ans = input(f"  {BOLD}>> {prompt} (y/n): {RESET}").strip().lower()
    return ans == "y"

def pause(prompt="Press Enter when ready..."):
    input(f"  {BOLD}>> {prompt}{RESET}")

def get_events(category):
    path = os.path.join(LOG_DIR, f"{category}.json")
    if not os.path.exists(path):
        return []
    try:
        with open(path) as f:
            return json.load(f).get("events", [])
    except Exception:
        return []

def wait_event(category, event_name, cond=None, timeout=30, quiet=False):
    if not quiet:
        print(f"  [LOG] Watching {category}.json for '{event_name}'…")
    base_len = len(get_events(category))
    for _ in range(timeout):
        time.sleep(1)
        evs = get_events(category)
        for e in evs[base_len:]:
            if e.get("event") == event_name:
                if cond is None or cond(e):
                    if not quiet:
                        print(f"  {GREEN}[LOG] Event '{event_name}' found.{RESET}")
                    return e
    if not quiet:
        print(f"  {YELLOW}[LOG] Timeout waiting for '{event_name}'.{RESET}")
    return None

# ── Main runner ────────────────────────────────────────────────────────────────

class RealDeviceTestRunner:
    def __init__(self):
        self.results = {}        # {tid: ("PASS"|"FAIL"|"SKIP", note)}
        self.system_prompt = ""

    # ── record helpers ─────────────────────────────────────────────────────────

    def record(self, tid, passed, note=""):
        status = "PASS" if passed else "FAIL"
        self.results[tid] = (status, note)
        (ok if passed else err)(f"{tid}: {note}")

    def manual(self, tid, prompt, note=""):
        passed = ask(prompt)
        self.record(tid, passed, note or prompt)
        return passed

    # ── section 1: boot & pre-check ───────────────────────────────────────────

    def test_boot(self):
        hdr("SECTION 1 — System Boot & Pre-Check (BC)")
        print("Ensure the application has just been launched (fresh start).")
        pause("Press Enter once the app is booting…")

        # BC1 — cold boot
        ev = wait_event("boot", "syscheck_complete", timeout=40)
        if ev:
            has_metrics = "cpu_pct" in ev and "ram_pct" in ev
            self.record("BC1", True,
                        f"syscheck_complete logged (P:{ev.get('pass')} W:{ev.get('warn')} F:{ev.get('fail')}) metrics={'yes' if has_metrics else 'missing'}")
            ui_ev = wait_event("ui", "state_transition",
                               lambda e: e.get("to") == "MainView", timeout=15)
            self.record("BC1_NAV", bool(ui_ev), "Auto-navigation to MainView after boot.")
        else:
            self.manual("BC1", "Did the Pre-Check screen complete and the app move to MainView?",
                        "User validated BC1 manually.")

        # BC2 — failure mode (manual only)
        print("\nBC2: Simulate a failure (e.g., kill a required ROS node) and restart the app.")
        self.manual("BC2", "Did the Pre-Check show red failures and remain locked?",
                    "Boot failure lockout validated manually.")

        # BC3 — manual nav launch
        print("\nBC3: In Control Center, tap Stop navigation then Start navigation again.")
        ev3 = wait_event("boot", "nav_process_started", timeout=30)
        self.record("BC3", bool(ev3), "nav_process_started logged." if ev3 else "Not found — manual check needed.")
        if not ev3:
            self.manual("BC3", "Did run_nav.sh execute and navigation restart?", "BC3 manual.")

    # ── section 2: direction view ─────────────────────────────────────────────

    def test_direction(self):
        hdr("SECTION 2 — Direction View & Destination Selection (DV)")
        print("Navigate to the Direction View on the robot screen.")
        pause()

        # DV1 — floor switch + popup reminder
        print("\nDV1: Tap the floor switch icon and select a different floor.")
        print("  Verify a reminder popup appears saying to restart navigation.")
        ok_popup = ask("Did the map background update and the restart-nav popup appear?")
        self.record("DV1", ok_popup, "Floor switch and restart popup verified.")

        # DV2 — room navigation confirmed
        print("\nDV2: Tap a checkpoint/room and confirm navigation ('Yes').")
        ev = wait_event("direction_view", "navigate_command", timeout=30)
        if ev:
            self.record("DV2", True, f"navigate_command logged (cp={ev.get('cpId')}).")
        else:
            self.manual("DV2", "Did the robot transition to RunningView after confirmation?",
                        "DV2 manual.")

        # DV3 — cancel confirmation
        print("\nDV3: Tap another checkpoint but press 'No'/'Cancel' in the popup.")
        self.manual("DV3", "Did the popup close and the app stay on DirectionView?", "DV3 manual.")

        # DV4 — auto-home timer
        print("\nDV4: Stay idle on DirectionView for ~15 seconds without touching the screen.")
        ev4 = wait_event("direction_view", "navigate_command",
                         lambda e: e.get("cpId") == 0, timeout=40)
        self.record("DV4", bool(ev4),
                    "Auto-home navigate_command (cpId=0) logged." if ev4
                    else "Not auto-detected — manual check.")
        if not ev4:
            self.manual("DV4", "Did the auto-home countdown popup appear and trigger navigation?",
                        "DV4 manual.")

    # ── section 3: running view ───────────────────────────────────────────────

    def test_running(self):
        hdr("SECTION 3 — Active Running View (RV)")
        print("Trigger navigation to a checkpoint and watch the RunningView.")
        pause("Press Enter once the robot is moving…")

        # RV1 — live progress
        ev = wait_event("running_view", "state_changed",
                        lambda e: e.get("to") == "moving", timeout=20)
        self.record("RV1", bool(ev),
                    "State changed to 'moving' logged with metrics." if ev
                    else "state_changed event not found.")

        # RV2 — emergency stop
        print("\nRV2: Tap the big red Stop button while the robot is moving.")
        ev_stop = wait_event("running_view", "emergency_stop",
                             lambda e: e.get("active") is True, timeout=20)
        self.record("RV2", bool(ev_stop),
                    "Emergency stop logged." if ev_stop else "Not found — manual check.")
        if not ev_stop:
            self.manual("RV2", "Did the robot halt and the button change to green Continue?",
                        "RV2 manual.")

        # RV3 — emergency resume
        print("\nRV3: Tap the green Continue button.")
        ev_res = wait_event("running_view", "emergency_stop",
                            lambda e: e.get("active") is False, timeout=20)
        self.record("RV3", bool(ev_res),
                    "Emergency resume logged." if ev_res else "Not found — manual check.")
        if not ev_res:
            self.manual("RV3", "Did the robot resume and button change back to red Stop?",
                        "RV3 manual.")

        # RV4 — reset mid-navigation
        print("\nRV4: Tap the Reset button while the robot is moving.")
        ev_reset = wait_event("running_view", "reset_requested", timeout=20)
        self.record("RV4", bool(ev_reset),
                    "reset_requested logged." if ev_reset else "Not found — manual check.")
        if not ev_reset:
            self.manual("RV4", "Did the robot halt and return to DirectionView?", "RV4 manual.")

        # RV5 — arrival
        print("\nRV5: Let the robot reach its target checkpoint (trigger nav again if needed).")
        ev_arr = wait_event("running_view", "checkpoint_arrived", timeout=60)
        self.record("RV5", bool(ev_arr),
                    f"checkpoint_arrived logged (id={ev_arr.get('id')})." if ev_arr
                    else "Not found — manual check.")
        if not ev_arr:
            self.manual("RV5", "Did the screen show 'Arrived' with a countdown timer?",
                        "RV5 manual.")

    # ── section 4: presentation view ─────────────────────────────────────────

    def test_presentation(self):
        hdr("SECTION 4 — Presentation & QR Code Upload (PV)")
        print("Navigate to the Presentation View from the main menu.")
        pause()

        # PV1 — QR code visible
        ev = wait_event("ui", "state_transition",
                        lambda e: e.get("to") == "PresentationView", timeout=20)
        self.record("PV1", bool(ev),
                    "State transition to PresentationView logged." if ev else "Not found — manual check.")
        if not ev:
            self.manual("PV1", "Is a QR code displayed on screen?", "PV1 manual.")

        # PV2 — upload .txt
        print("\nPV2: Scan the QR code from a phone and upload a .txt file.")
        self.manual("PV2", "Did the robot screen display the uploaded text content?", "PV2 manual.")

        # PV3 — upload .pptx
        print("\nPV3: Upload a .pptx PowerPoint file via the QR link on your phone.")
        self.manual("PV3", "Did the slide viewer become active with the first slide?", "PV3 manual.")

        # PV4 — slide controls
        print("\nPV4: Use the Next and Previous slide controls.")
        self.manual("PV4", "Did the slides advance/go back and the indicator update (e.g. 2/10)?",
                    "PV4 manual.")

    # ── section 5: control center & diagnostics ───────────────────────────────

    def test_control_center(self):
        hdr("SECTION 5 — Control Center & Diagnostics (CC)")
        print("Ensure you are logged in as User (default). Do NOT switch roles yet.")
        pause()

        # CC1 — access protection (still User)
        self.manual("CC1",
                    "As a regular User, does attempting to open Control Center show an access denied/login prompt?",
                    "CC1 access gate verified.")

        # Elevate to Admin for CC2 / CC3
        print("\nElevate role: Settings → Switch Mode → Administrator → enter password.")
        pause("Press Enter once you are logged in as Administrator…")
        ev_admin = wait_event("ui", "state_transition",
                              lambda e: e.get("to") == "ControlCenterView", timeout=20)

        # CC2 — live performance graphs
        print("\nCC2: Open Diagnostics View and watch the graphs for ~10 seconds.")
        ev_diag = wait_event("ui", "state_transition",
                             lambda e: e.get("to") == "DiagnosticsView", timeout=20)
        ev_perf = wait_event("control_center", "perf_sample", timeout=15)
        self.record("CC2", bool(ev_diag and ev_perf),
                    "DiagnosticsView loaded and perf_sample logged." if (ev_diag and ev_perf)
                    else "Manual check required.")
        if not (ev_diag and ev_perf):
            self.manual("CC2", "Do CPU, RAM, FPS, and Network graphs update in real-time?",
                        "CC2 manual.")

        # CC3 — low FPS alert (manual stress test)
        print("\nCC3: Apply CPU stress (e.g., 'stress --cpu 4') and watch the Diagnostics graph.")
        self.manual("CC3",
                    "Did the FPS drop below 15 and the graph reflect the drop?",
                    "CC3 low-FPS alert manual.")

    # ── section 6: access control & wifi settings ─────────────────────────────

    def test_settings(self):
        hdr("SECTION 6 — Access Control & Wi-Fi Settings (US)")

        # US1 — admin elevation
        print("US1: Settings → Switch Mode → Admin → enter password.")
        ev_badge = wait_event("ui", "state_transition",
                              lambda e: "Settings" in e.get("from", ""), timeout=30)
        self.manual("US1",
                    "Is the 'Admin' badge visible in the top-right corner after login?",
                    "US1 role elevation verified.")

        # US2 — idle logout
        print("\nUS2: Remain logged in as Admin but do NOT touch the screen for 3 minutes.")
        self.manual("US2",
                    "After 3 minutes, did the badge revert to 'User' automatically?",
                    "US2 idle timeout verified.")

        # US3 — wifi scan & connect
        print("\nUS3: Settings → Wi-Fi → Scan, select a network, enter password.")
        self.manual("US3",
                    "Did the network list populate and the selected network connect successfully?",
                    "US3 Wi-Fi connection verified.")

    # ── chatbot tests ─────────────────────────────────────────────────────────
    # Chatbot tests are in a dedicated script: run_chatbot_tests.py
    # Run: python3 run_chatbot_tests.py

    # ── report ────────────────────────────────────────────────────────────────

    def report(self):
        hdr("REAL DEVICE TEST REPORT")
        total = len(self.results)
        passed = sum(1 for s, _ in self.results.values() if s == "PASS")
        failed = total - passed

        print(f"\n  {BOLD}Total: {total}  |  {GREEN}PASS: {passed}{RESET}  |  {RED}FAIL: {failed}{RESET}\n")

        sections = [
            ("Boot & Pre-Check",  ["BC1", "BC1_NAV", "BC2", "BC3"]),
            ("Direction View",    ["DV1", "DV2", "DV3", "DV4"]),
            ("Running View",      ["RV1", "RV2", "RV3", "RV4", "RV5"]),
            ("Presentation View", ["PV1", "PV2", "PV3", "PV4"]),
            ("Control Center",    ["CC1", "CC2", "CC3"]),
            ("Settings / Access", ["US1", "US2", "US3"]),
        ]

        report_lines = ["# ASIC Mobile Robot — Real Device Test Report\n\n"]
        report_lines.append(f"**Total: {total}  PASS: {passed}  FAIL: {failed}**\n\n")

        for sec_name, tids in sections:
            print(f"  {BOLD}{sec_name}{RESET}")
            report_lines.append(f"\n## {sec_name}\n\n")
            report_lines.append("| ID | Status | Note |\n|---|---|---|\n")
            for tid in tids:
                if tid in self.results:
                    status, note = self.results[tid]
                    c = GREEN if status == "PASS" else RED
                    print(f"    {tid:<14} {c}{status}{RESET}  {note}")
                    report_lines.append(f"| {tid} | {status} | {note} |\n")

        os.makedirs("artifacts", exist_ok=True)
        path = "artifacts/real_device_test_report.md"
        with open(path, "w") as f:
            f.writelines(report_lines)
        print(f"\n  {BOLD}Report saved to: {path}{RESET}")

    # ── entry point ───────────────────────────────────────────────────────────

    def run(self):
        hdr("ASIC MOBILE ROBOT — REAL DEVICE INTERACTIVE TEST SUITE (BC/DV/RV/PV/CC/US)")
        print("Covers hardware & UI tests. Chatbot tests → python3 run_chatbot_tests.py")
        print("The app must already be running on the device.")
        print("Log events are monitored automatically; manual y/n prompts are used as fallback.")
        pause("Press Enter to begin…")

        self.test_boot()
        self.test_direction()
        self.test_running()
        self.test_presentation()
        self.test_control_center()
        self.test_settings()
        self.report()


if __name__ == "__main__":
    RealDeviceTestRunner().run()
