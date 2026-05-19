# Implementation Plan: RPAL Interpreter (PLAN_TICKET.md)

## Phase 0: Project Setup
- [ ] **Step 0.1:** Initialize the C++ project structure in the `src/` directory.
- [ ] **Step 0.2:** Create the `Makefile` in the root directory to compile the source code into the `rpal20` executable.
- [ ] **Step 0.3:** Define base core data structures: `Token` and `TreeNode`.

## Phase 1: Lexical Analysis (Scanner)
- [ ] **Step 1.1:** Implement the `Lexer` class capable of robust file I/O and lookahead.
- [ ] **Step 1.2:** Implement tokenization logic strictly according to `RPAL_LEXICON.md`, categorizing sequences into identifiers, integers, strings, operators, and punctuation.
- [ ] **Step 1.3:** Ensure whitespace and comments (mapped as `<DELETE>`) are correctly ignored.
- [ ] **Step 1.4:** Test the `Lexer` in isolation to ensure a 100% correct token stream.

## Phase 2: Syntax Analysis (Parser & AST Generation)
- [ ] **Step 2.1:** Implement the `Parser` class utilizing the `Lexer`.
- [ ] **Step 2.2:** Implement Recursive Descent parsing functions (e.g., `E()`, `T()`, `Ta()`) mapped directly to the Phrase Structure Grammar in `RPAL_GRAMMAR.md`.
- [ ] **Step 2.3:** Dynamically construct the Abstract Syntax Tree (AST) using n-ary `TreeNode` objects.
- [ ] **Step 2.4:** Implement an AST print function (e.g. `ast -ast`) for manual verification.

## Phase 3: Standardization (ST Generation)
- [ ] **Step 3.1:** Implement the `Standardizer` class.
- [ ] **Step 3.2:** Implement Sub-Tree Transformational Grammar rules (`AST_STANDARDIZATION_RULES.md`) to desugar the AST.
- [ ] **Step 3.3:** Ensure the resulting Standardized Tree (ST) correctly outputs a binary tree format utilizing exclusively `gamma` and `lambda` internal nodes.

## Phase 4: CSE Machine Evaluation
- [ ] **Step 4.1:** Implement data structures for the CSE Machine: `Environment` (for variable mapping), `Stack` (for operands), and `Control` (for operation sequence).
- [ ] **Step 4.2:** Implement ST flattening to generate initial control sequences.
- [ ] **Step 4.3:** Implement the `CSEMachine` evaluation loop evaluating RPAL operational semantics based strictly on `CSE_MACHINE_RULES.md`.
- [ ] **Step 4.4:** Integrate built-in functions (e.g., `Print`, `Stem`, `Stern`, `Conc`, arithmetic/boolean ops).

## Phase 5: Pipeline Integration & Verification
- [ ] **Step 5.1:** Develop `main.cpp` tying the sequential pipeline together: Lexer -> Parser -> Standardizer -> CSEMachine.
- [ ] **Step 5.2:** Implement integration testing using `bash_execute` against `rpal_test_programs/`. Verify the output format exactly matches `./rpal.exe <file>`.
- [ ] **Step 5.3:** Perform final code quality reviews, check for memory leaks, ensure comments exist, and confirm `make` works flawlessly.
