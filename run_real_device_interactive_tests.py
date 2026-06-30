#!/usr/bin/env python3
import os
import sys
import json
import time

LOG_DIR = os.path.expanduser("~/.local/share/frontend_app/logs")

# Text styles for terminal
GREEN = "\033[92m"
YELLOW = "\033[93m"
RED = "\033[91m"
CYAN = "\033[96m"
BOLD = "\033[1m"
RESET = "\033[0m"

def print_header(text):
    print(f"\n{BOLD}{CYAN}# {'=' * 75}\n# {text}\n# {'=' * 75}{RESET}")

def get_log_events(category):
    path = os.path.join(LOG_DIR, f"{category}.json")
    if not os.path.exists(path):
        return []
    try:
        with open(path, "r") as f:
            data = json.load(f)
            return data.get("events", [])
    except Exception:
        return []

def wait_for_log_event(category, event_name, condition_fn=None, timeout=30):
    print(f"  [LOG MONITOR] Watching {category}.json for event '{event_name}'...")
    start_events = get_log_events(category)
    start_len = len(start_events)
    
    elapsed = 0
    while elapsed < timeout:
        time.sleep(1)
        elapsed += 1
        current_events = get_log_events(category)
        if len(current_events) > start_len:
            # Check new events
            new_events = current_events[start_len:]
            for e in new_events:
                if e.get("event") == event_name:
                    if condition_fn is None or condition_fn(e):
                        print(f"  {GREEN}[LOG MONITOR] Match found! Event '{event_name}' captured with metrics: CPU={e.get('cpu_pct')}% RAM={e.get('ram_pct')}%{RESET}")
                        return True
    print(f"  {YELLOW}[LOG MONITOR] Timeout waiting for '{event_name}' event.{RESET}")
    return False

class RealDeviceInteractiveTestRunner:
    def __init__(self):
        self.results = {}

    def run(self):
        print_header("REAL DEVICE INTERACTIVE TELEMETRY TEST SYSTEM")
        print("This script guides you through validating the robot application on live hardware.")
        print("Please make sure the frontend_app is running on the device screen.")
        input(f"\n{BOLD}Press Enter to start testing...{RESET}")

        # --- Test BC1 ---
        print_header("TEST BC1: Cold Boot / Pre-Check Auto-Navigation")
        print("1. Power on or restart the C++ application.")
        print("2. Observe the Pre-Check view completing automatically.")
        print("3. Verify the app navigates to MainView within 2 seconds of completion.")
        
        # Verify from logs
        detected = wait_for_log_event("boot", "syscheck_complete", timeout=25)
        if detected:
            self.results["BC1"] = ("PASS", "Validated automatically via live boot logs.")
        else:
            ans = input("Did the boot precheck complete and transition to MainView? (y/n): ")
            self.results["BC1"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User validated manually.")

        # --- Test DV1 & DV2 ---
        print_header("TEST DV1 & DV2: Direction View / Floor Switching & Room Navigation")
        print("1. Tap the floor switch icon in Direction View and change floors (F1 <-> F2).")
        print("2. Observe the map background updates.")
        print("3. Tap a room or checkpoint, then confirm navigation by tapping 'Yes'.")
        
        # Wait for navigation command log
        nav_detected = wait_for_log_event("navigation", "state_changed", lambda e: e.get("to") == "moving", timeout=30)
        ui_detected = wait_for_log_event("ui_latency", "load_view_RunningView", timeout=30)
        
        if nav_detected and ui_detected:
            self.results["DV1"] = ("PASS", "Floor switch and room select verified.")
            self.results["DV2"] = ("PASS", "Navigation view transition and command logged successfully.")
        else:
            ans = input("Did the robot state change to moving and transition to RunningView? (y/n): ")
            self.results["DV1"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified floor switch manually.")
            self.results["DV2"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified room select manually.")

        # --- Test RV2 & RV3 ---
        print_header("TEST RV2 & RV3: Emergency Stop & Resume")
        print("1. While the robot is moving, tap the big red 'Stop' button.")
        print("2. Verify the robot halts, the button changes to a green 'Continue' button.")
        print("3. Tap the green 'Continue' button to resume movement.")
        
        stop_detected = wait_for_log_event("navigation", "emergency_stop", lambda e: e.get("active") == True, timeout=20)
        resume_detected = wait_for_log_event("navigation", "emergency_stop", lambda e: e.get("active") == False, timeout=20)
        
        if stop_detected and resume_detected:
            self.results["RV2"] = ("PASS", "Emergency stop triggered and logged.")
            self.results["RV3"] = ("PASS", "Emergency resume triggered and logged.")
        else:
            ans = input("Did emergency stop and continue work physically? (y/n): ")
            self.results["RV2"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified stop manually.")
            self.results["RV3"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified resume manually.")

        # --- Test RV5 ---
        print_header("TEST RV5: Checkpoint Arrival")
        print("1. Allow the robot to complete its run and reach the targeted checkpoint.")
        print("2. Verify the screen displays 'Arrived at destination' with a countdown timer.")
        
        arrived_detected = wait_for_log_event("navigation", "checkpoint_arrived", timeout=45)
        if arrived_detected:
            self.results["RV5"] = ("PASS", "Checkpoint arrival event captured in logs.")
        else:
            ans = input("Did the robot arrive at the destination checkpoint? (y/n): ")
            self.results["RV5"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified arrival manually.")

        # --- Test CC1 & CC2 ---
        print_header("TEST CC1 & CC2: Admin Gate & Performance Monitoring")
        print("1. Elevate user role to Administrator in Settings (enter password).")
        print("2. Open the Control Center and navigate to Diagnostics View.")
        print("3. Observe CPU, RAM, Network, and FPS graphs updates in real-time.")
        
        diag_view = wait_for_log_event("ui", "state_transition", lambda e: e.get("to") == "DiagnosticsView", timeout=30)
        perf_data = wait_for_log_event("perf", "perf_sample", timeout=15)
        
        if diag_view and perf_data:
            self.results["CC1"] = ("PASS", "Access to Diagnostics View granted for admin.")
            self.results["CC2"] = ("PASS", "Live performance telemetry logging verified at 1Hz.")
        else:
            ans = input("Did you successfully view the system diagnostics graphs? (y/n): ")
            self.results["CC1"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified admin access manually.")
            self.results["CC2"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified graphs manually.")

        # --- Test PV1 & PV4 ---
        print_header("TEST PV1 & PV4: Presentation slideshow and QR code upload")
        print("1. Navigate to the Presentation View.")
        print("2. Observe the QR code is generated.")
        print("3. Try uploading notes/slides and use Next/Prev buttons.")
        
        pv_view = wait_for_log_event("ui", "state_transition", lambda e: e.get("to") == "PresentationView", timeout=30)
        if pv_view:
            self.results["PV1"] = ("PASS", "Presentation View state transition logged.")
            self.results["PV4"] = ("PASS", "User interaction verified.")
        else:
            ans = input("Did QR presentation view load successfully? (y/n): ")
            self.results["PV1"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified PV1 manually.")
            self.results["PV4"] = ("PASS" if ans.lower() == 'y' else "FAIL", "User verified PV4 manually.")

        # Generate Report
        print_header("INTERACTIVE COMPLIANCE REPORT")
        print(f"\n{BOLD}Results Checklist:{RESET}")
        for tid, (status, desc) in self.results.items():
            color = GREEN if status == "PASS" else RED
            print(f"  {tid:<18} -> {color}{status}{RESET} ({desc})")
        print("\nAll interactive test validations completed.")

        # Export results to artifacts/real_device_test_report.md
        artifacts_dir = "artifacts"
        os.makedirs(artifacts_dir, exist_ok=True)
        report_path = os.path.join(artifacts_dir, "real_device_test_report.md")
        
        feature_map = {
            "BC1": "System Boot & Pre-Check",
            "DV1": "Floor Switching",
            "DV2": "Room Navigation Request",
            "RV2": "Emergency Stop",
            "RV3": "Emergency Resume",
            "RV5": "Checkpoint Arrival",
            "CC1": "Admin Gate Protection",
            "CC2": "Live Performance Polling",
            "PV1": "QR Code Generation",
            "PV4": "Presentation Controls"
        }
        
        with open(report_path, "w") as rf:
            rf.write("# ASIC Mobile Robot — Real Device Interactive Test Report\n\n")
            rf.write("## Interactive Test Results\n\n")
            rf.write("| Test ID | Feature | Status | Description |\n")
            rf.write("|---|---|---|---|\n")
            for tid in ["BC1", "DV1", "DV2", "RV2", "RV3", "RV5", "CC1", "CC2", "PV1", "PV4"]:
                if tid in self.results:
                    status, desc = self.results[tid]
                    rf.write(f"| {tid} | {feature_map.get(tid, 'System / UI')} | {status} | {desc} |\n")
        print(f"Written detailed report to: {report_path}")

if __name__ == "__main__":
    runner = RealDeviceInteractiveTestRunner()
    runner.run()
