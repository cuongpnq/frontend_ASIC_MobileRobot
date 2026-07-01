# ASIC Bot — Chatbot Test Scenarios

> **Knowledge base**: `knowledge.txt` (about UIT/FCE/app)  
> **Session log**: `~/.local/share/frontend_app/logs/chat.json`  
> Events to look for: `cat: "chat"` → events `query_start`, `first_token`, `complete`

---

## Scoring Rubric

| Score | Meaning |
|---|---|
| ✅ 2 | Fully correct, concise, right language |
| ⚠️ 1 | Partially correct or slightly off |
| ❌ 0 | Wrong, hallucinated, or wrong language |

**Pass threshold per category: ≥ 75% of questions score ≥ 1**

---

## Performance Targets (read from `complete` event in log)

| Metric | Acceptable | Good | Excellent |
|---|---|---|---|
| `ttft_ms` (Time to First Token) | < 3000 ms | < 1500 ms | < 800 ms |
| `tokens_per_sec` | > 8 | > 15 | > 25 |
| `total_ms` for typical answer | < 15 s | < 8 s | < 4 s |

---

## Category 1 — Factual / In-Scope (English)

*These questions have direct answers in `knowledge.txt`. Score 0 if hallucinated.*

| # | Question to type | Expected answer / keywords | Score |
|---|---|---|---|
| F1 | `When was UIT established?` | June 8, 2006 | |
| F2 | `What is the full name of UIT?` | University of Information Technology, VNU-HCM | |
| F3 | `Where is UIT located?` | Thu Duc City / Linh Trung Ward, Ho Chi Minh City | |
| F4 | `Who is the dean of the Faculty of Computer Engineering?` | Dr. Nguyen Minh Son | |
| F5 | `What is the email of the dean?` | sonnm@uit.edu.vn | |
| F6 | `Who are the vice deans of FCE?` | Dr. Doan Duy + MSc. Phan Dinh Duy | |
| F7 | `What does FCE focus on?` | embedded systems, hardware design, computer architecture | |
| F8 | `Who developed ASIC Bot?` | ASIC Laboratory at UIT | |
| F9 | `What is the role of ASIC Bot?` | navigation + guidance at UIT FCE | |
| F10 | `What model does the chatbot use?` | Ollama / local language model (acceptable: any LLM reference) | |

---

## Category 2 — Factual / In-Scope (Vietnamese)

*Same facts, asked in Vietnamese. The bot MUST reply in Vietnamese (check language).*

| # | Question to type | Expected keywords (Vietnamese) | Score |
|---|---|---|---|
| V1 | `Trường UIT thành lập năm nào?` | 8 tháng 6, 2006 | |
| V2 | `Tên đầy đủ của trường UIT là gì?` | Đại học Công nghệ Thông tin, ĐHQG TP.HCM | |
| V3 | `Khoa Kỹ thuật Máy tính chuyên về lĩnh vực gì?` | hệ thống nhúng, kiến trúc máy tính | |
| V4 | `Trưởng khoa KTMt là ai?` | TS. Nguyễn Minh Sơn | |
| V5 | `Robot này do ai phát triển?` | Phòng thí nghiệm ASIC | |
| V6 | `Làm thế nào để chuyển sang chế độ quản trị viên?` | Cài đặt → Chuyển chế độ → nhập mật khẩu | |

---

## Category 3 — Application Usage Guide

*The bot should correctly explain how to use the touch-screen application.*

| # | Question to type | Expected content | Score |
|---|---|---|---|
| A1 | `How do I send the robot to a room?` | Direction View → tap checkpoint on map | |
| A2 | `How do I switch between floors?` | Direction View → Switch button (top-right) | |
| A3 | `How do I upload a presentation file?` | Presentation View → scan QR code | |
| A4 | `What file formats can I upload for presentation?` | .pptx, .txt | |
| A5 | `How do I access the Control Center?` | Requires Administrator role | |
| A6 | `What does the Diagnostics view show?` | CPU, RAM, FPS, network, disk | |
| A7 | `How do I send the robot home?` | Direction View → Home button / tap Home checkpoint | |
| A8 | `How long until the system logs me out?` | 3 minutes of inactivity | |
| A9 | `What are the user access roles?` | User, Administrator, Developer | |
| A10 | `How do I stop the robot while it is moving?` | RunningView → Stop button (Emergency Stop) | |

---

## Category 4 — Out-of-Scope Rejection

*The bot should politely refuse or redirect — NOT hallucinate an answer.*

| # | Question to type | Expected behavior | Score |
|---|---|---|---|
| O1 | `What is the capital of France?` | Politely says it's outside its knowledge, offers to help with UIT/nav | |
| O2 | `Write me a Python function to sort a list.` | Redirects, does not write arbitrary code | |
| O3 | `Who is the president of UIT?` | Admits it doesn't know (not in knowledge.txt) | |
| O4 | `What is the wifi password?` | Admits it doesn't have that info | |
| O5 | `Can you book a meeting room for me?` | Explains it cannot perform that action | |

---

## Category 5 — Edge / Adversarial

*Tests robustness against unusual or tricky inputs.*

| # | Question to type | Expected behavior | Score |
|---|---|---|---|
| E1 | *(empty message — just tap Send with blank input)* | No response sent (UI should block) | |
| E2 | `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa` | Graceful response or polite confusion | |
| E3 | `WHEN WAS UIT ESTABLISHED?` (all caps) | Correct answer — June 8, 2006 | |
| E4 | `Tell me everything you know.` | Structured summary of knowledge, no hallucination | |
| E5 | `Are you ChatGPT?` | Correctly identifies itself as ASIC Bot, not ChatGPT | |

---

## Category 6 — Multi-Turn Conversation

*Tests context retention across turns (max history = 6 messages = 3 turns).*

Run these **in sequence without clearing history**:

| # | Turn | Message | Expected | Score |
|---|---|---|---|---|
| M1 | Turn 1 | `Who is the dean of FCE?` | Dr. Nguyen Minh Son | |
| M2 | Turn 2 | `What is his email?` | sonnm@uit.edu.vn (uses "his" from context) | |
| M3 | Turn 3 | `And the vice dean?` | Dr. Doan Duy / MSc. Phan Dinh Duy | |
| M4 | Turn 4 | `Tell me about the robot.` | ASIC Bot navigation info | |
| M5 | Turn 5 | `How do I use it?` | Direction View explanation (uses "it" from context) | |
| M6 | Turn 6 | `Who made it?` | ASIC Laboratory (maintained across 3+ turns) | |

---

## Category 7 — Performance Benchmark

*Run these after a cold boot (no prior chat session) to get clean TTFT numbers.*

| # | Scenario | What to measure in log |
|---|---|---|
| P1 | Short question: `What is UIT?` | `ttft_ms` and `total_ms` |
| P2 | Long answer: `Describe all the views in the application.` | `tokens_per_sec` and `token_count` |
| P3 | Follow-up (warm context): `Tell me more about Direction View.` | `ttft_ms` vs P1 (should be similar or lower) |
| P4 | Stop mid-response (tap Stop button) | `interrupted: true` in log, `token_count` = partial |
| P5 | Ask same question twice in a row | Both `ttft_ms` values, check consistency |

---

## How to Read Results from Session Log

```bash
# Show all chat events from chat.json log
cat ~/.local/share/frontend_app/logs/chat.json \
  | python3 -c "
import json, sys
data = json.load(sys.stdin)
for e in data['events']:
    if e.get('cat') == 'chat':
        print(e)
"
```

### Example log entries you will see:

```json
{ "t": "...", "cat": "chat", "event": "query_start",
  "question": "When was UIT established?", "history_turns": 0 }

{ "t": "...", "cat": "chat", "event": "first_token",
  "ttft_ms": 743.2 }

{ "t": "...", "cat": "chat", "event": "complete",
  "question": "When was UIT established?",
  "token_count": 24,
  "total_ms": 1820.5,
  "tokens_per_sec": 13.18,
  "interrupted": false,
  "answer_len_chars": 87 }
```

### Quick analysis script:

```python
import json, os, statistics

path = os.path.expanduser("~/.local/share/frontend_app/logs/chat.json")
if os.path.exists(path):
    with open(path) as f:
        data = json.load(f)

    ttfts   = [e["ttft_ms"]       for e in data.get("events", [])
               if e.get("cat") == "chat" and e.get("event") == "first_token"]
    tps     = [e["tokens_per_sec"] for e in data.get("events", [])
               if e.get("cat") == "chat" and e.get("event") == "complete"]

    print(f"\n=== {path} ===")
    if ttfts: print(f"  TTFT   avg={statistics.mean(ttfts):.0f}ms  min={min(ttfts):.0f}ms  max={max(ttfts):.0f}ms")
    if tps:   print(f"  tok/s  avg={statistics.mean(tps):.1f}  min={min(tps):.1f}  max={max(tps):.1f}")
```

---

## Test Execution Checklist

- `[ ]` Robot powered on, navigation running
- `[ ]` Ollama service confirmed running (`curl http://localhost:11434/`)
- `[ ]` Open ChatView on touchscreen
- `[ ]` Run Category 1 (F1–F10) — record scores
- `[ ]` Run Category 2 (V1–V6) — check language of response
- `[ ]` Run Category 3 (A1–A10) — check application guide accuracy
- `[ ]` Run Category 4 (O1–O5) — verify no hallucination
- `[ ]` Run Category 5 (E1–E5) — check edge case handling
- `[ ]` Clear history, run Category 6 (M1–M6) in sequence
- `[ ]` Cold boot, run Category 7 (P1–P5) — capture from log
- `[ ]` Run analysis script on session log
- `[ ]` Document final score per category
