# ARCHITECTURE.md: RPAL Interpreter System State

## 1. System Overview

This project implements a complete interpreter for the RPAL (Right-reference Pedagogic Algorithmic Language) programming language.

* **Target Executable:** `rpal20`
* **Language/Stack:** C++ (Manual memory management, OOP paradigm).
* **Execution Paradigm:** `./rpal20 <file_name>`
* **Primary Objective:** The output of `rpal20` must perfectly match the output of the reference `rpal.exe` for any given test program.

## 2. Invariants & Hard Constraints

* **NO GENERATORS:** The use of `lex`, `yacc`, `flex`, `bison`, or any automated parser generator is strictly prohibited. The Lexer and Parser must be written from scratch.
* **PARSING STRATEGY:** The Parser MUST be implemented using a Recursive Descent strategy, mapping directly to the Phrase Structure Grammar provided in `RPAL_GRAMMAR.md`.
* **COMPILATION:** The project must compile via a `Makefile` located at the root of the directory. The default `make` command must produce the `rpal20` executable.

## 3. Interpreter Pipeline & Data Flow

The interpreter operates in a strictly sequential, four-stage pipeline. Worker Agents must maintain strict separation of concerns between these modules.

### Phase A: Lexical Analysis (Scanner)

* **Input:** Raw RPAL source code file.
* **Output:** A stream of `Token` objects.
* **Responsibilities:** Scans characters, ignores white space and comments (classified as `<DELETE>`), and identifies identifiers, integers, strings, operators, and punctuation according to `RPAL_LEXICON.md`.

### Phase B: Syntax Analysis (Parser & AST Generation)

* **Input:** Stream of `Token` objects.
* **Output:** An Abstract Syntax Tree (AST).
* **Responsibilities:** Implements Recursive Descent parsing. For each grammar rule (e.g., `E -> 'let' D 'in' E`), the parser consumes tokens and builds an n-ary tree structure.

### Phase C: Standardization (ST Generation)

* **Input:** Abstract Syntax Tree (AST).
* **Output:** Standardized Tree (ST).
* **Responsibilities:** "Desugars" the AST by applying the Sub-Tree Transformational Grammar rules. The resulting ST must be a binary tree where internal nodes are exclusively `gamma` and `lambda`.

### Phase D: CSE Machine Evaluation

* **Input:** Standardized Tree (ST).
* **Output:** Standard output (console).
* **Responsibilities:** Flattens the ST into a control structure sequence. Initializes the **C**ontrol, **S**tack, and **E**nvironment (PE - Primitive Environment). Executes the operational semantics of RPAL to evaluate the program and print the final result.

## 4. Proposed C++ Object Model

Worker Agents should adhere to the following baseline class architecture to maintain modularity:

* `Token`: Holds string value, token type enum, and line number.
* `Lexer`: Reads file I/O, manages lookahead, and exposes `getNextToken()`.
* `TreeNode`: Represents an AST/ST node (pointers to children/siblings, node type, value).
* `Parser`: Consumes Lexer, contains recursive functions for `E()`, `T()`, `Ta()`, etc., and builds `TreeNode`s.
* `Standardizer`: Contains the tree mutation logic (e.g., `standardizeLet()`, `standardizeWhere()`).
* `CSEMachine`: Manages `Environment` (variable mappings), `Stack` (operands), and `Control` (operations sequence). Contains the `evaluate()` loop.

## 5. Agent Instructions for PLAN_TICKET Generation

When the Planner Agent reads this document to generate `PLAN_TICKET.md`, it must segment the tasks strictly by the 4 pipeline phases. It must mandate sandboxed `bash_execute` testing using the `rpal_test_programs/` directory after each major module completion.
