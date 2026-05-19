# Antigravity Orchestrator: Global Instructions & Constraints

## 1. Project Objective & Core Constraints

The objective is to build an RPAL interpreter consisting of a Lexical Analyzer, a Recursive Descent Parser (generating an AST), a Standardizer (generating an ST), and a CSE Machine evaluator.

* **Tech Stack:** C++ (compiled via `g++` and `make`).
* **Forbidden Tools:** The use of `lex`, `yacc`, or any automated parser generators is STRICTLY PROHIBITED.
* **Grading Compliance:** Code MUST be highly readable and well-commented. The Critic Agent must reject undocumented code.

## 2. The 4-Phase Antigravity Workflow

All agentic operations must strictly adhere to the deterministic 4-Phase pipeline.

### Phase 1: Planning & Alignment

* The **Planner Agent** reads the user prompt and `ARCHITECTURE.md`.
* It outputs a highly granular, step-by-step implementation plan to `PLAN_TICKET.md`.
* **CONSTRAINT [HITL 1]:** The Orchestrator MUST pause execution. The human operator must review, modify, and explicitly approve `PLAN_TICKET.md` before any C++ code generation begins.

### Phase 2: Autonomous Worker Execution

* For each step in `PLAN_TICKET.md`, a **Worker Agent** is spawned with access to the explicit context cache.
* **Lexer/Parser Tasks:** Must rigidly follow the Recursive Descent methodology outlined in the lecture notes.
* **Self-Verification:** The Worker MUST use the `run_command` tool to compile the C++ code via WSL (`wsl make`) and run provided test cases in WSL (`wsl ./rpal20 rpal_test_programs/rpal_XX > output.XX`) before submitting a code diff.

### Phase 3: Critic Review (LLM-as-a-Judge)

* The **Critic Agent** evaluates the Worker's submitted diff.
* **Evaluation Vectors:**
  1. *Intent Alignment:* Does it strictly implement the current step in `PLAN_TICKET.md`?
  2. *Constraint Check:* Is it written in C++? Is it free of `lex`/`yacc`? Is it implemented via Recursive Descent?
  3. *Quality Assurance:* Are the functions documented? Are memory leaks mitigated?
* If rejected, the Critic provides actionable feedback, and the Worker loops. If approved, proceed to Phase 4.

### Phase 4: Merge, Sync, and Version Control

* **CONSTRAINT [HITL 2]:** The Orchestrator MUST pause. The human operator reviews the Critic-approved diffs.
* Upon human approval, the Orchestrator applies the diffs to the C++ files.
* **Knowledge Transfer:** The Planner Agent updates `ARCHITECTURE.md` to reflect the newly implemented logic.
* **Version Control Checkpoint:** The Orchestrator MUST execute the following Git commands to snapshot the clean state:
  `git add .`
  `git commit -m "feat: Completed [Step Name] from PLAN_TICKET - Critic Approved"`
