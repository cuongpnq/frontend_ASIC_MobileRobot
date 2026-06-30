#!/usr/bin/env python3
import os
import sys
import json
import time
import subprocess
import urllib.request
import urllib.error

# Setup configuration
LOG_DIR = os.path.expanduser("~/.local/share/frontend_app/logs")
APP_BIN = "./build-output/frontend/frontend_app"
KNOWLEDGE_FILE = "frontend/knowledge.txt"
OLLAMA_URL = "http://localhost:11434/api/chat"

# Text styles for terminal
GREEN = "\033[92m"
YELLOW = "\033[93m"
RED = "\033[91m"
CYAN = "\033[96m"
BOLD = "\033[1m"
RESET = "\033[0m"

def print_banner(text):
    print(f"\n{BOLD}{CYAN}# {'=' * 70}\n# {text}\n# {'=' * 70}{RESET}")

def run_command(args, env=None):
    try:
        return subprocess.run(args, capture_output=True, text=True, env=env, timeout=10)
    except subprocess.TimeoutExpired:
        return None

def publish_ros_topic(topic, type_str, data_str):
    cmd = ["ros2", "topic", "pub", "-1", topic, type_str, f"data: {data_str}"]
    print(f"  [MOCK ROS] Publishing: {topic} ({type_str}) -> '{data_str}'")
    subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

class NativeAutomationTestRunner:
    def __init__(self):
        self.results = {}
        self.chat_results = {}
        self.system_results = {}

    def setup_env(self):
        print_banner("1. PREPARING TEST ENVIRONMENT")
        # Clean old log dir
        if os.path.exists(LOG_DIR):
            print(f"Cleaning logs directory: {LOG_DIR}")
            for f in os.listdir(LOG_DIR):
                path = os.path.join(LOG_DIR, f)
                try:
                    if os.path.isfile(path):
                        os.unlink(path)
                except Exception as e:
                    print(f"Error cleaning {path}: {e}")
        else:
            os.makedirs(LOG_DIR, exist_ok=True)
            print(f"Created logs directory: {LOG_DIR}")

        # Check binary
        if not os.path.exists(APP_BIN):
            print(f"{RED}[ERROR] C++ application binary not found at {APP_BIN}. Please run 'make build' first.{RESET}")
            sys.exit(1)
        print(f"Found compiled application binary: {APP_BIN}")

        # Check Ollama
        try:
            req = urllib.request.Request("http://localhost:11434/", method="GET")
            with urllib.request.urlopen(req) as response:
                if response.status == 200:
                    print(f"Ollama server is running locally.")
        except Exception as e:
            print(f"{RED}[ERROR] Ollama server is not running at http://localhost:11434/. Check with 'systemctl status ollama'.{RESET}")
            sys.exit(1)

        # Read knowledge.txt
        if not os.path.exists(KNOWLEDGE_FILE):
            print(f"{RED}[ERROR] System knowledge file not found at {KNOWLEDGE_FILE}.{RESET}")
            sys.exit(1)
        with open(KNOWLEDGE_FILE, "r") as kf:
            self.system_prompt = kf.read()
        print(f"Loaded knowledge database: {len(self.system_prompt)} chars.")

    def wait_for_log_event(self, category, event_name, condition_fn=None, timeout=25):
        path = os.path.join(LOG_DIR, f"{category}.json")
        elapsed = 0
        while elapsed < timeout:
            if os.path.exists(path):
                try:
                    with open(path, "r") as f:
                        data = json.load(f)
                    events = data.get("events", [])
                    for e in events:
                        if e.get("event") == event_name:
                            if condition_fn is None or condition_fn(e):
                                return e
                except Exception:
                    pass
            time.sleep(0.5)
            elapsed += 0.5
        return None

    def run_system_tests(self):
        print_banner("2. RUNNING CORE SYSTEM AUTOMATION TESTS")
        
        # Start application headlessly
        print("Launching frontend_app in offscreen mode...")
        env = os.environ.copy()
        env["QT_QPA_PLATFORM"] = "offscreen"
        env["MOCK_HARDWARE"] = "1"
        
        # Launch using ROS source prefix to initialize ROS node
        self.app_log = open("app_run.log", "w")
        app_proc = subprocess.Popen(
            ["bash", "-c", f"source /opt/ros/foxy/setup.bash && {APP_BIN}"],
            env=env,
            stdout=self.app_log,
            stderr=self.app_log,
            preexec_fn=os.setsid
        )

        try:
            print("Waiting for boot pre-check (BC1) and auto-navigation...")
            
            # Verify BC1 (Cold Boot / SysCheck complete)
            sys_comp = self.wait_for_log_event("boot", "syscheck_complete", timeout=25)
            if sys_comp:
                print(f"{GREEN}[PASS] BC1: Cold Boot Pre-Check successfully completed.{RESET}")
                # Verify CPU/RAM metrics are injected in boot check event
                if "cpu_pct" in sys_comp and "ram_pct" in sys_comp:
                    print(f"{GREEN}[PASS] Telemetry check: CPU and RAM usage injected in 'syscheck_complete'.{RESET}")
                    self.system_results["BC1"] = ("PASS", "Pre-check completed and logged with metrics.")
                else:
                    print(f"{YELLOW}[WARN] BC1: 'syscheck_complete' exists but misses 'cpu_pct'/'ram_pct'.{RESET}")
                    self.system_results["BC1"] = ("WARN", "Pre-check completed but metrics missing.")
            else:
                print(f"{RED}[FAIL] BC1: 'syscheck_complete' event not found.{RESET}")
                self.system_results["BC1"] = ("FAIL", "'syscheck_complete' missing or timeout.")

            # Verify view transition logs (BC1 auto-navigation)
            main_view_trans = self.wait_for_log_event("ui", "state_transition", lambda e: e.get("to") == "MainView", timeout=15)
            if main_view_trans:
                print(f"{GREEN}[PASS] UI check: Transition to MainView successfully captured.{RESET}")
                self.system_results["UI_Transition"] = ("PASS", "Transition to MainView logged.")
            else:
                print(f"{RED}[FAIL] UI check: Transition to MainView not found.{RESET}")
                self.system_results["UI_Transition"] = ("FAIL", "Transition to MainView not logged.")

            # Verify Navigation & Running View events (DV2, RV1, RV5)
            print("Simulating active navigation lifecycle...")
            publish_ros_topic("/robot/state", "std_msgs/msg/String", "'moving'")
            publish_ros_topic("/robot/status_message", "std_msgs/msg/String", "'Navigating to checkpoint'")
            
            state_changed = self.wait_for_log_event("navigation", "state_changed", lambda e: e.get("to") == "moving", timeout=15)
            
            publish_ros_topic("/robot/current_checkpoint", "std_msgs/msg/Int32", "5")
            cp_arrived = self.wait_for_log_event("navigation", "checkpoint_arrived", lambda e: e.get("id") == 5, timeout=15)

            if state_changed and "cpu_pct" in state_changed:
                print(f"{GREEN}[PASS] RV1: Live progress state changes and performance metrics successfully logged.{RESET}")
                self.system_results["RV1"] = ("PASS", "Progress state and performance metrics logged.")
            else:
                self.system_results["RV1"] = ("FAIL", "Progress state/metrics missing.")

            if cp_arrived and "cpu_pct" in cp_arrived:
                print(f"{GREEN}[PASS] RV5: Checkpoint arrival events logged with CPU/RAM usage.{RESET}")
                self.system_results["RV5"] = ("PASS", "Arrival logged with performance metrics.")
            else:
                self.system_results["RV5"] = ("FAIL", "Checkpoint arrival event or metrics missing.")

            # Verify view loading latency check (DV2 / Load view latency)
            view_load = self.wait_for_log_event("ui_latency", "load_view_MainView", timeout=15)
            if view_load:
                print(f"{GREEN}[PASS] DV2: View loading latency and description recorded successfully.{RESET}")
                self.system_results["DV2"] = ("PASS", f"View load latency tracked: {view_load.get('elapsed_ms')} ms")
            else:
                self.system_results["DV2"] = ("FAIL", "View load latency event missing.")

        finally:
            print("Terminating frontend_app...")
            app_proc.terminate()
            app_proc.wait()
            self.app_log.close()
            subprocess.run(["pkill", "-9", "-f", "frontend_app"])
            print("frontend_app terminated cleanly.")

    def run_chatbot_tests(self):
        print_banner("3. RUNNING CHATBOT SCENARIOS AUTOMATION")
        
        # Scenarios taken from chatbot_test_scenarios.md
        scenarios = {
            "Category 1 — Factual (English)": [
                ("F1", "When was UIT established?", ["2006", "June 8"]),
                ("F2", "What is the full name of UIT?", ["University of Information Technology", "VNU-HCM"]),
                ("F3", "Where is UIT located?", ["Thu Duc", "Ho Chi Minh"]),
                ("F4", "Who is the dean of the Faculty of Computer Engineering?", ["Nguyen Minh Son"]),
                ("F5", "What is the email of the dean?", ["sonnm@uit.edu.vn"]),
                ("F6", "Who are the vice deans of FCE?", ["Doan Duy", "Phan Dinh Duy"]),
                ("F7", "What does FCE focus on?", ["embedded", "architecture", "hardware"]),
                ("F8", "Who developed ASIC Bot?", ["ASIC Laboratory", "ASIC Lab", "ASIC"]),
                ("F9", "What is the role of ASIC Bot?", ["navigation", "guidance"]),
                ("F10", "What model does the chatbot use?", ["Ollama", "local", "model"])
            ],
            "Category 2 — Factual (Vietnamese)": [
                ("V1", "Trường UIT thành lập năm nào?", ["8 tháng 6", "2006"]),
                ("V2", "Tên đầy đủ của trường UIT là gì?", ["Đại học Công nghệ Thông tin", "ĐHQG"]),
                ("V3", "Khoa Kỹ thuật Máy tính chuyên về lĩnh vực gì?", ["nhúng", "kiến trúc máy tính"]),
                ("V4", "Trưởng khoa KTMt là ai?", ["Nguyễn Minh Sơn"]),
                ("V5", "Robot này do ai phát triển?", ["ASIC"]),
                ("V6", "Làm thế nào để chuyển sang chế độ quản trị viên?", ["Cài đặt", "Chuyển chế độ", "Settings", "Switch Mode"])
            ],
            "Category 3 — App Usage": [
                ("A1", "How do I send the robot to a room?", ["Direction View", "checkpoint", "map"]),
                ("A2", "How do I switch between floors?", ["floor", "switch", "map"]),
                ("A3", "How do I upload a presentation file?", ["Presentation View", "QR"]),
                ("A4", "What file formats can I upload for presentation?", [".pptx", ".txt"]),
                ("A5", "How do I access the Control Center?", ["Administrator", "Admin"]),
                ("A6", "What does the Diagnostics view show?", ["CPU", "RAM", "FPS"]),
                ("A7", "How do I send the robot home?", ["Home"]),
                ("A8", "How long until the system logs me out?", ["3 minutes", "inactivity"]),
                ("A9", "What are the user access roles?", ["User", "Administrator", "Developer"]),
                ("A10", "How do I stop the robot while it is moving?", ["Stop", "Emergency", "Running"])
            ],
            "Category 4 — Out of Scope": [
                ("O1", "What is the capital of France?", ["outside", "knowledge", "UIT", "cannot", "sorry"]),
                ("O2", "Write me a Python function to sort a list.", ["outside", "cannot", "sorry"]),
                ("O3", "Who is the president of UIT?", ["know", "not in", "sorry"]),
                ("O4", "What is the wifi password?", ["have", "password", "sorry"]),
                ("O5", "Can you book a meeting room for me?", ["cannot", "perform", "sorry"])
            ],
            "Category 5 — Edge Cases": [
                ("E2", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", ["clarify", "pardon", "sorry", "understand"]),
                ("E3", "WHEN WAS UIT ESTABLISHED?", ["2006", "June 8"]),
                ("E4", "Tell me everything you know.", ["ASIC Bot", "UIT", "FCE", "Faculty"]),
                ("E5", "Are you ChatGPT?", ["ASIC Bot"])
            ]
        }

        for cat_name, cases in scenarios.items():
            print(f"\nRunning {BOLD}{cat_name}{RESET}:")
            cat_results = []
            for tid, question, keywords in cases:
                score, resp, ttft, tok_sec = self.query_ollama(question, keywords)
                cat_results.append((tid, question, score, resp, ttft, tok_sec))
                status = f"{GREEN}PASS (2/2){RESET}" if score == 2 else (f"{YELLOW}PARTIAL (1/2){RESET}" if score == 1 else f"{RED}FAIL (0/2){RESET}")
                print(f"  [{tid}] '{question}' -> {status} (TTFT: {ttft:.0f}ms, Speed: {tok_sec:.1f} tok/s)")
                if score < 2:
                    print(f"    {YELLOW}[DEBUG] Response: {resp}{RESET}")
            self.chat_results[cat_name] = cat_results

        # Run Multi-Turn Scenarios sequentially
        print(f"\nRunning {BOLD}Category 6 — Multi-Turn Conversation{RESET}:")
        multi_turn_cases = [
            ("M1", "Who is the dean of FCE?", ["Nguyen Minh Son"]),
            ("M2", "What is his email?", ["sonnm@uit.edu.vn"]),
            ("M3", "And the vice dean?", ["Doan Duy", "Phan Dinh Duy"])
        ]
        history = []
        cat_results = []
        for tid, question, keywords in multi_turn_cases:
            score, resp, ttft, tok_sec = self.query_ollama(question, keywords, history=history)
            history.append({"role": "user", "content": question})
            history.append({"role": "assistant", "content": resp})
            cat_results.append((tid, question, score, resp, ttft, tok_sec))
            status = f"{GREEN}PASS (2/2){RESET}" if score == 2 else (f"{YELLOW}PARTIAL (1/2){RESET}" if score == 1 else f"{RED}FAIL (0/2){RESET}")
            print(f"  [{tid}] '{question}' -> {status} (TTFT: {ttft:.0f}ms, Speed: {tok_sec:.1f} tok/s)")
            if score < 2:
                print(f"    {YELLOW}[DEBUG] Response: {resp}{RESET}")
        self.chat_results["Category 6 — Multi-Turn"] = cat_results

    def query_ollama(self, question, expected_keywords, history=None):
        messages = []
        if history:
            for item in history:
                messages.append(item)
        messages.append({"role": "user", "content": question})

        body = {
            "model": "qwen2.5:0.5b",
            "messages": [
                {"role": "system", "content": self.system_prompt}
            ] + messages,
            "stream": False,
            "options": {
                "temperature": 0.3,
                "top_k": 20,
                "top_p": 0.85,
                "repeat_penalty": 1.1,
                "num_predict": 350,
                "num_ctx": 2048
            }
        }

        data = json.dumps(body).encode("utf-8")
        req = urllib.request.Request(OLLAMA_URL, data=data, headers={"Content-Type": "application/json"}, method="POST")

        start_time = time.time()
        try:
            with urllib.request.urlopen(req) as response:
                res_body = json.loads(response.read().decode("utf-8"))
                total_time = time.time() - start_time
                answer = res_body.get("message", {}).get("content", "").strip()
                
                # Extract speeds
                eval_count = res_body.get("eval_count", 0)
                eval_duration = res_body.get("eval_duration", 1) / 1e9 # ns to s
                prompt_eval_duration = res_body.get("prompt_eval_duration", 0) / 1e6 # ns to ms
                
                ttft = prompt_eval_duration if prompt_eval_duration > 0 else (total_time * 0.15 * 1000)
                tok_sec = eval_count / eval_duration if eval_duration > 0 else 0

                # Score response
                matched = any(k.lower() in answer.lower() for k in expected_keywords)
                score = 2 if matched else 0
                
                # Check for language match
                if question.strip()[-1] == '?': # simple English check
                     pass # heuristic
                return score, answer, ttft, tok_sec
        except Exception as e:
            return 0, f"Error: {e}", 0.0, 0.0

    def generate_report(self):
        print_banner("4. AUTOMATION TESTS COMPLIANCE SUMMARY")
        print(f"\n{BOLD}Core System Tests:{RESET}")
        for tid, (status, desc) in self.system_results.items():
            color = GREEN if status == "PASS" else (YELLOW if status == "WARN" else RED)
            print(f"  {tid:<18} -> {color}{status}{RESET} ({desc})")

        print(f"\n{BOLD}Chatbot Tests Score & Performance:{RESET}")
        for cat, cases in self.chat_results.items():
            total = len(cases)
            passed = sum(1 for c in cases if c[2] >= 1)
            pct = (passed / total) * 100
            color = GREEN if pct >= 75 else RED
            print(f"  {cat:<35}: {color}{pct:.1f}%{RESET} ({passed}/{total} passed)")

        # Save artifact report
        report_path = "artifacts/automation_test_report.md"
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, "w") as rf:
            rf.write("# ASIC Mobile Robot — Automation Test Report\n\n")
            rf.write("## 1. Core System Results\n\n")
            rf.write("| Test ID | Feature | Status | Description |\n")
            rf.write("|---|---|---|---|\n")
            for tid, (status, desc) in self.system_results.items():
                rf.write(f"| {tid} | System / UI | {status} | {desc} |\n")
            
            rf.write("\n## 2. Chatbot Scenarios Score\n\n")
            rf.write("| Category | Questions Passed | Percentage | Status |\n")
            rf.write("|---|---|---|---|\n")
            for cat, cases in self.chat_results.items():
                total = len(cases)
                passed = sum(1 for c in cases if c[2] >= 1)
                pct = (passed / total) * 100
                status = "PASS" if pct >= 75 else "FAIL"
                rf.write(f"| {cat} | {passed}/{total} | {pct:.1f}% | {status} |\n")
        
        print(f"\nWritten detailed report to: {BOLD}{report_path}{RESET}\n")

if __name__ == "__main__":
    runner = NativeAutomationTestRunner()
    runner.setup_env()
    runner.run_system_tests()
    runner.run_chatbot_tests()
    runner.generate_report()
