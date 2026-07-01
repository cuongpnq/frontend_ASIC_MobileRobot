#!/usr/bin/env python3
"""
ASIC Mobile Robot — Chatbot Test Script
Covers all 7 categories from chatbot_test_scenarios.md:
  Cat 1  — Factual / In-Scope (English)
  Cat 2  — Factual / In-Scope (Vietnamese)
  Cat 3  — Application Usage Guide
  Cat 4  — Out-of-Scope Rejection
  Cat 5  — Edge / Adversarial Cases
  Cat 6  — Multi-Turn Conversation
  Cat 7  — Performance Benchmark

Requirements:
  - Ollama running at localhost:11434  (systemctl status ollama)
  - knowledge.txt present at  frontend/knowledge.txt
  - Model qwen2.5:0.5b pulled        (ollama pull qwen2.5:0.5b)

Usage:
  python3 run_chatbot_tests.py            # all categories
  python3 run_chatbot_tests.py --cat 1    # single category
  python3 run_chatbot_tests.py --cat 4 5  # multiple categories
"""

import os
import sys
import json
import time
import glob
import statistics
import argparse
import urllib.request
import urllib.error

# ── Config ─────────────────────────────────────────────────────────────────────
KNOWLEDGE_FILE = "frontend/knowledge.txt"
OLLAMA_URL     = "http://localhost:11434/api/chat"
OLLAMA_MODEL   = "qwen2.5:0.5b"
LOG_DIR        = os.path.expanduser("~/.local/share/frontend_app/logs")
REPORT_PATH    = "artifacts/chatbot_test_report.md"

PASS_THRESHOLD = 0.75   # 75 % of questions must score ≥ 1

# Performance targets (ms / tok/s)
TTFT_ACCEPTABLE = 3000
TTFT_GOOD       = 1500
TTFT_EXCELLENT  = 800
TPS_ACCEPTABLE  = 8
TPS_GOOD        = 15
TPS_EXCELLENT   = 25

# ── Terminal colours ────────────────────────────────────────────────────────────
GREEN  = "\033[92m"
YELLOW = "\033[93m"
RED    = "\033[91m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
DIM    = "\033[2m"
RESET  = "\033[0m"

def hdr(text):
    print(f"\n{BOLD}{CYAN}# {'=' * 72}\n# {text}\n# {'=' * 72}{RESET}")

def tag(passed, partial=False):
    if passed:   return f"{GREEN}PASS ✓{RESET}"
    if partial:  return f"{YELLOW}PARTIAL ⚠{RESET}"
    return f"{RED}FAIL ✗{RESET}"

def ttft_label(ms):
    if ms < TTFT_EXCELLENT:  return f"{GREEN}{ms:.0f} ms (excellent){RESET}"
    if ms < TTFT_GOOD:       return f"{GREEN}{ms:.0f} ms (good){RESET}"
    if ms < TTFT_ACCEPTABLE: return f"{YELLOW}{ms:.0f} ms (acceptable){RESET}"
    return f"{RED}{ms:.0f} ms (slow){RESET}"

def tps_label(tps):
    if tps > TPS_EXCELLENT:  return f"{GREEN}{tps:.1f} tok/s (excellent){RESET}"
    if tps > TPS_GOOD:       return f"{GREEN}{tps:.1f} tok/s (good){RESET}"
    if tps > TPS_ACCEPTABLE: return f"{YELLOW}{tps:.1f} tok/s (acceptable){RESET}"
    return f"{RED}{tps:.1f} tok/s (slow){RESET}"

# ── Ollama helpers ──────────────────────────────────────────────────────────────

def check_ollama():
    try:
        with urllib.request.urlopen("http://localhost:11434/") as r:
            return r.status == 200
    except Exception:
        return False

def chat(system_prompt, question, history=None):
    """
    Send a single-turn or multi-turn request to Ollama.
    Returns (answer: str, ttft_ms: float, tok_per_sec: float, total_ms: float).
    """
    messages = list(history or [])
    messages.append({"role": "user", "content": question})

    body = json.dumps({
        "model": OLLAMA_MODEL,
        "messages": [{"role": "system", "content": system_prompt}] + messages,
        "stream": False,
        "options": {
            "temperature": 0.3,
            "top_k": 20,
            "top_p": 0.85,
            "repeat_penalty": 1.1,
            "num_predict": 400,
            "num_ctx": 2048,
        },
    }).encode()

    req = urllib.request.Request(
        OLLAMA_URL, data=body,
        headers={"Content-Type": "application/json"}, method="POST"
    )
    t0 = time.time()
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            res = json.loads(r.read())
        total_ms   = (time.time() - t0) * 1000
        answer     = res.get("message", {}).get("content", "").strip()
        eval_count = res.get("eval_count", 0)
        eval_dur   = res.get("eval_duration", 1) / 1e9          # ns → s
        prompt_dur = res.get("prompt_eval_duration", 0) / 1e6   # ns → ms
        ttft_ms    = prompt_dur if prompt_dur > 0 else total_ms * 0.15
        tok_s      = eval_count / eval_dur if eval_dur > 0 else 0.0
        return answer, ttft_ms, tok_s, total_ms
    except Exception as e:
        return f"ERROR: {e}", 0.0, 0.0, 0.0

def score(answer, keywords):
    """Return 2 if all keywords found, 1 if any, 0 if none."""
    low = answer.lower()
    hits = sum(1 for k in keywords if k.lower() in low)
    if hits == len(keywords): return 2
    if hits > 0:              return 1
    return 0

# ── Test runner class ───────────────────────────────────────────────────────────

class ChatbotTestRunner:

    def __init__(self, system_prompt: str):
        self.sp       = system_prompt
        self.results  = {}   # tid → {score, answer, ttft, tps, total_ms}
        self.cat_info = {}   # cat_id → (name, [tid, ...])

    # ── internal query wrapper ─────────────────────────────────────────────────

    def _run(self, tid, question, keywords, history=None, silent=False):
        answer, ttft, tps, total = chat(self.sp, question, history)
        sc = score(answer, keywords)
        self.results[tid] = {
            "score": sc, "answer": answer,
            "ttft": ttft, "tps": tps, "total_ms": total,
            "question": question, "keywords": keywords,
        }
        if not silent:
            passed  = sc == 2
            partial = sc == 1
            print(f"    [{tid}] {tag(passed, partial)}  TTFT={ttft:.0f}ms  {tps:.1f} tok/s")
            print(f"           Q: {question[:80]}")
            if sc < 2:
                snippet = answer[:160].replace("\n", " ")
                print(f"           {YELLOW}A: {snippet}…{RESET}")
        return sc

    # ── Category 1 — Factual English ──────────────────────────────────────────

    def cat1(self):
        hdr("Category 1 — Factual / In-Scope (English)")
        cases = [
            ("F1",  "When was UIT established?",
             ["2006", "June 8"]),
            ("F2",  "What is the full name of UIT?",
             ["University of Information Technology", "VNU-HCM"]),
            ("F3",  "Where is UIT located?",
             ["Thu Duc", "Ho Chi Minh"]),
            ("F4",  "Who is the dean of the Faculty of Computer Engineering?",
             ["Nguyen Minh Son"]),
            ("F5",  "What is the email of the dean?",
             ["sonnm@uit.edu.vn"]),
            ("F6",  "Who are the vice deans of FCE?",
             ["Doan Duy", "Phan Dinh Duy"]),
            ("F7",  "What does FCE focus on?",
             ["embedded", "architecture", "hardware"]),
            ("F8",  "Who developed ASIC Bot?",
             ["ASIC Laboratory", "ASIC Lab", "ASIC"]),
            ("F9",  "What is the role of ASIC Bot?",
             ["navigation", "guidance"]),
            ("F10", "What model does the chatbot use?",
             ["Ollama", "local", "model"]),
        ]
        self.cat_info[1] = ("Factual English", [c[0] for c in cases])
        for tid, q, kws in cases:
            self._run(tid, q, kws)

    # ── Category 2 — Factual Vietnamese ──────────────────────────────────────

    def cat2(self):
        hdr("Category 2 — Factual / In-Scope (Vietnamese)")
        cases = [
            ("V1", "Trường UIT thành lập năm nào?",
             ["2006", "8 tháng 6"]),
            ("V2", "Tên đầy đủ của trường UIT là gì?",
             ["Công nghệ Thông tin", "ĐHQG"]),
            ("V3", "Khoa Kỹ thuật Máy tính chuyên về lĩnh vực gì?",
             ["nhúng", "kiến trúc"]),
            ("V4", "Trưởng khoa KTMt là ai?",
             ["Nguyễn Minh Sơn"]),
            ("V5", "Robot này do ai phát triển?",
             ["ASIC"]),
            ("V6", "Làm thế nào để chuyển sang chế độ quản trị viên?",
             ["Cài đặt", "Switch Mode", "Chuyển chế độ"]),
        ]
        self.cat_info[2] = ("Factual Vietnamese", [c[0] for c in cases])
        for tid, q, kws in cases:
            self._run(tid, q, kws)

    # ── Category 3 — App Usage ────────────────────────────────────────────────

    def cat3(self):
        hdr("Category 3 — Application Usage Guide")
        cases = [
            ("A1",  "How do I send the robot to a room?",
             ["Direction View", "checkpoint"]),
            ("A2",  "How do I switch between floors?",
             ["floor", "switch", "map"]),
            ("A3",  "How do I upload a presentation file?",
             ["Presentation View", "QR"]),
            ("A4",  "What file formats can I upload for presentation?",
             [".pptx", ".txt"]),
            ("A5",  "How do I access the Control Center?",
             ["Administrator", "Admin"]),
            ("A6",  "What does the Diagnostics view show?",
             ["CPU", "RAM", "FPS"]),
            ("A7",  "How do I send the robot home?",
             ["Home"]),
            ("A8",  "How long until the system logs me out?",
             ["3 minutes", "inactivity"]),
            ("A9",  "What are the user access roles?",
             ["User", "Administrator", "Developer"]),
            ("A10", "How do I stop the robot while it is moving?",
             ["Stop", "Emergency"]),
        ]
        self.cat_info[3] = ("App Usage", [c[0] for c in cases])
        for tid, q, kws in cases:
            self._run(tid, q, kws)

    # ── Category 4 — Out-of-Scope Rejection ──────────────────────────────────

    def cat4(self):
        hdr("Category 4 — Out-of-Scope Rejection (bot must refuse)")
        cases = [
            ("O1", "What is the capital of France?",
             ["outside", "cannot", "sorry", "knowledge"]),
            ("O2", "Write me a Python function to sort a list.",
             ["outside", "cannot", "sorry"]),
            ("O3", "Who is the president of UIT?",
             ["know", "sorry", "not in", "outside"]),
            ("O4", "What is the wifi password?",
             ["password", "sorry", "have"]),
            ("O5", "Can you book a meeting room for me?",
             ["cannot", "sorry", "perform"]),
        ]
        self.cat_info[4] = ("Out-of-Scope", [c[0] for c in cases])
        for tid, q, kws in cases:
            self._run(tid, q, kws)

    # ── Category 5 — Edge / Adversarial ──────────────────────────────────────

    def cat5(self):
        hdr("Category 5 — Edge / Adversarial Cases")
        cases = [
            ("E1", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
             ["clarify", "pardon", "sorry", "understand", "help"]),
            ("E2", "WHEN WAS UIT ESTABLISHED?",
             ["2006", "June 8"]),
            ("E3", "Tell me everything you know.",
             ["ASIC Bot", "UIT", "navigation"]),
            ("E4", "Are you ChatGPT?",
             ["ASIC Bot"]),
            ("E5", "     ",
             ["help", "UIT", "navigation", "sorry"]),  # whitespace-only
        ]
        self.cat_info[5] = ("Edge Cases", [c[0] for c in cases])
        for tid, q, kws in cases:
            self._run(tid, q, kws)

    # ── Category 6 — Multi-Turn Conversation ─────────────────────────────────

    def cat6(self):
        hdr("Category 6 — Multi-Turn Conversation (context retention)")
        print(f"  {DIM}Running turns in sequence without resetting history.{RESET}\n")
        turns = [
            ("M1", "Who is the dean of FCE?",
             ["Nguyen Minh Son"]),
            ("M2", "What is his email?",
             ["sonnm@uit.edu.vn"]),
            ("M3", "And the vice dean?",
             ["Doan Duy", "Phan Dinh Duy"]),
            ("M4", "Tell me about the robot.",
             ["ASIC Bot", "navigation"]),
            ("M5", "How do I use it?",
             ["Direction View", "checkpoint"]),
            ("M6", "Who made it?",
             ["ASIC Laboratory", "ASIC Lab", "ASIC"]),
        ]
        self.cat_info[6] = ("Multi-Turn", [t[0] for t in turns])
        history = []
        for tid, q, kws in turns:
            self._run(tid, q, kws, history=history)
            r = self.results[tid]
            history.append({"role": "user",      "content": q})
            history.append({"role": "assistant", "content": r["answer"]})
            if len(history) > 12:   # keep last 6 turns
                history = history[-12:]

    # ── Category 7 — Performance Benchmark ───────────────────────────────────

    def cat7(self):
        hdr("Category 7 — Performance Benchmark")
        print(f"  {DIM}Run after a cold boot for clean TTFT numbers.{RESET}\n")
        bench = [
            ("P1", "What is UIT?",
             ["University of Information Technology"]),
            ("P2", "Describe all the views in the application.",
             ["Direction View", "Presentation View", "Control Center"]),
            ("P3", "Tell me more about Direction View.",
             ["checkpoint", "floor", "map"]),
            ("P4", "How do I access the Control Center?",  # repeated question
             ["Administrator", "Admin"]),
            ("P5", "How do I access the Control Center?",  # same question twice → check consistency
             ["Administrator", "Admin"]),
        ]
        self.cat_info[7] = ("Performance", [b[0] for b in bench])

        ttfts, tpss = [], []
        for tid, q, kws in bench:
            answer, ttft, tps, total = chat(self.sp, q)
            sc = score(answer, kws)
            self.results[tid] = {
                "score": sc, "answer": answer,
                "ttft": ttft, "tps": tps, "total_ms": total,
                "question": q, "keywords": kws,
            }
            ttfts.append(ttft)
            tpss.append(tps)
            print(f"    [{tid}]  TTFT={ttft_label(ttft)}  Speed={tps_label(tps)}  total={total:.0f}ms")
            print(f"           Q: {q[:80]}")

        if len(ttfts) >= 2:
            print(f"\n  {BOLD}Benchmark summary:{RESET}")
            print(f"    TTFT  avg={statistics.mean(ttfts):.0f}ms  "
                  f"min={min(ttfts):.0f}ms  max={max(ttfts):.0f}ms")
            print(f"    tok/s avg={statistics.mean(tpss):.1f}  "
                  f"min={min(tpss):.1f}  max={max(tpss):.1f}")

        # Consistency check for P4 vs P5
        p4, p5 = self.results.get("P4"), self.results.get("P5")
        if p4 and p5:
            delta = abs(p4["ttft"] - p5["ttft"])
            consistent = delta < 500
            c = GREEN if consistent else YELLOW
            print(f"    Repeat TTFT delta: {c}{delta:.0f}ms{RESET} "
                  f"({'consistent' if consistent else 'variable'})")

    # ── Session log analyser ───────────────────────────────────────────────────

    def analyse_session_logs(self):
        hdr("Session Log Analysis (chat events from device logs)")
        files = glob.glob(os.path.join(LOG_DIR, "chat.json"))
        if not files:
            print(f"  {YELLOW}No chat.json found in {LOG_DIR}.{RESET}")
            print(f"  {DIM}Run the app on the device, open ChatView, ask some questions, then re-run this script.{RESET}")
            return

        for path in files:
            try:
                with open(path) as f:
                    data = json.load(f)
            except Exception as e:
                print(f"  {RED}Could not parse {path}: {e}{RESET}")
                continue

            events = data.get("events", [])
            completions = [e for e in events if e.get("event") == "complete"]
            first_tokens = [e for e in events if e.get("event") == "first_token"]

            if not completions:
                print(f"  {YELLOW}No 'complete' events in {path}.{RESET}")
                continue

            ttfts = [e["ttft_ms"] for e in first_tokens if "ttft_ms" in e]
            tpss  = [e["tokens_per_sec"] for e in completions if "tokens_per_sec" in e]
            tots  = [e["total_ms"] for e in completions if "total_ms" in e]

            print(f"\n  {BOLD}{path}{RESET}")
            print(f"    Queries completed : {len(completions)}")
            if ttfts:
                print(f"    TTFT  avg={statistics.mean(ttfts):.0f}ms  "
                      f"min={min(ttfts):.0f}ms  max={max(ttfts):.0f}ms")
            if tpss:
                print(f"    tok/s avg={statistics.mean(tpss):.1f}  "
                      f"min={min(tpss):.1f}  max={max(tpss):.1f}")
            if tots:
                print(f"    total avg={statistics.mean(tots):.0f}ms  "
                      f"min={min(tots):.0f}ms  max={max(tots):.0f}ms")

    # ── Summary report ─────────────────────────────────────────────────────────

    def report(self, selected_cats):
        hdr("CHATBOT TEST REPORT")

        # ── Per-category summary ──────────────────────────────────────────────
        cat_rows = []
        overall_pass = overall_total = 0

        for cat_id, (cat_name, tids) in sorted(self.cat_info.items()):
            scores  = [self.results[t]["score"] for t in tids if t in self.results]
            passed  = sum(1 for s in scores if s >= 1)
            total   = len(scores)
            pct     = (passed / total * 100) if total else 0
            verdict = "PASS" if pct >= PASS_THRESHOLD * 100 else "FAIL"
            c       = GREEN if verdict == "PASS" else RED
            print(f"  Cat {cat_id} ({cat_name:<22}) : {c}{pct:5.1f}%{RESET}  "
                  f"({passed}/{total})  {c}{verdict}{RESET}")
            cat_rows.append((cat_id, cat_name, passed, total, pct, verdict))
            overall_pass  += passed
            overall_total += total

        overall_pct = (overall_pass / overall_total * 100) if overall_total else 0
        c = GREEN if overall_pct >= PASS_THRESHOLD * 100 else RED
        print(f"\n  {BOLD}Overall: {c}{overall_pct:.1f}%{RESET}  ({overall_pass}/{overall_total})\n")

        # ── Performance stats ─────────────────────────────────────────────────
        ttfts = [r["ttft"] for r in self.results.values() if r["ttft"] > 0]
        tpss  = [r["tps"]  for r in self.results.values() if r["tps"]  > 0]
        if ttfts:
            print(f"  TTFT   avg={statistics.mean(ttfts):.0f}ms  "
                  f"min={min(ttfts):.0f}ms  max={max(ttfts):.0f}ms")
        if tpss:
            print(f"  tok/s  avg={statistics.mean(tpss):.1f}  "
                  f"min={min(tpss):.1f}  max={max(tpss):.1f}")

        # ── Failed cases detail ───────────────────────────────────────────────
        failed = {t: r for t, r in self.results.items() if r["score"] < 2}
        if failed:
            print(f"\n  {BOLD}Failed / Partial cases:{RESET}")
            for tid, r in failed.items():
                label = "PARTIAL" if r["score"] == 1 else "FAIL"
                c     = YELLOW   if r["score"] == 1 else RED
                print(f"    [{tid}] {c}{label}{RESET}  Q: {r['question'][:70]}")
                print(f"           A: {r['answer'][:120].replace(chr(10), ' ')}")

        # ── Write Markdown report ─────────────────────────────────────────────
        os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
        ts = time.strftime("%Y-%m-%d %H:%M")

        with open(REPORT_PATH, "w") as f:
            f.write(f"# ASIC Bot — Chatbot Test Report\n\n")
            f.write(f"**Generated:** {ts}  |  **Model:** `{OLLAMA_MODEL}`\n\n")
            f.write(f"**Overall: {overall_pct:.1f}%  ({overall_pass}/{overall_total})**\n\n")

            f.write("## Category Summary\n\n")
            f.write("| Cat | Name | Passed | Total | % | Verdict |\n")
            f.write("|---|---|---|---|---|---|\n")
            for cat_id, cat_name, p, t, pct, verdict in cat_rows:
                f.write(f"| {cat_id} | {cat_name} | {p} | {t} | {pct:.1f}% | {verdict} |\n")

            f.write("\n## Per-Question Results\n\n")
            f.write("| ID | Question | Score | TTFT (ms) | tok/s |\n")
            f.write("|---|---|---|---|---|\n")
            for tid, r in self.results.items():
                sc_str = "✅" if r["score"] == 2 else ("⚠️" if r["score"] == 1 else "❌")
                f.write(f"| {tid} | {r['question'][:60]} | {sc_str} | "
                        f"{r['ttft']:.0f} | {r['tps']:.1f} |\n")

            if failed:
                f.write("\n## Failed / Partial Detail\n\n")
                for tid, r in failed.items():
                    f.write(f"### [{tid}] {r['question']}\n\n")
                    f.write(f"- **Score:** {r['score']}/2\n")
                    f.write(f"- **Expected keywords:** {r['keywords']}\n")
                    f.write(f"- **Answer:** {r['answer'][:400]}\n\n")

        print(f"\n  {BOLD}Report saved: {REPORT_PATH}{RESET}")


# ── CLI entry point ─────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="ASIC Bot Chatbot Test Runner")
    parser.add_argument(
        "--cat", nargs="*", type=int,
        choices=[1, 2, 3, 4, 5, 6, 7],
        metavar="N",
        help="Categories to run (1-7). Omit to run all.")
    parser.add_argument(
        "--logs-only", action="store_true",
        help="Only analyse session logs from the device; skip Ollama queries.")
    args = parser.parse_args()

    selected = args.cat if args.cat else list(range(1, 8))

    hdr("ASIC BOT — CHATBOT TEST SUITE")
    print(f"  Model    : {BOLD}{OLLAMA_MODEL}{RESET}")
    print(f"  Knowledge: {KNOWLEDGE_FILE}")
    print(f"  Cats     : {selected}")
    print(f"  Report   : {REPORT_PATH}\n")

    # ── Pre-flight checks ──────────────────────────────────────────────────────
    if not args.logs_only:
        if not check_ollama():
            print(f"{RED}[ERROR] Ollama not reachable at http://localhost:11434/\n"
                  f"        Start it with: systemctl start ollama{RESET}")
            sys.exit(1)
        print(f"  {GREEN}Ollama: OK{RESET}")

        if not os.path.exists(KNOWLEDGE_FILE):
            print(f"{RED}[ERROR] knowledge.txt not found at '{KNOWLEDGE_FILE}'{RESET}")
            sys.exit(1)
        with open(KNOWLEDGE_FILE) as f:
            system_prompt = f.read()
        print(f"  {GREEN}Knowledge loaded: {len(system_prompt)} chars{RESET}\n")

    # ── Run ────────────────────────────────────────────────────────────────────
    if args.logs_only:
        runner = ChatbotTestRunner("")
        runner.analyse_session_logs()
        return

    runner = ChatbotTestRunner(system_prompt)

    cat_map = {
        1: runner.cat1,
        2: runner.cat2,
        3: runner.cat3,
        4: runner.cat4,
        5: runner.cat5,
        6: runner.cat6,
        7: runner.cat7,
    }

    for cat_id in selected:
        cat_map[cat_id]()

    # Always show session-log stats if available
    runner.analyse_session_logs()
    runner.report(selected)


if __name__ == "__main__":
    main()
