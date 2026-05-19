# C++ RPAL Interpreter

This repository contains a complete, robust C++ implementation of an interpreter for the **Recursive Program Association Language (RPAL)**, achieving 100% behavioral parity with the reference RPAL execution environment.

RPAL is a functional programming language designed to demonstrate compiler concepts. This interpreter is structured as a classical compiler frontend and interpreter backend, utilizing a Control Stack Evaluator (CSE) Machine for execution.

---

## Codebase Architecture & File Structure

The project is structured logically into components under the `src/` directory. Each file is accompanied by a dedicated markdown explanation to aid learning:

```text
rpal-interpreter/
├── src/
│   ├── Token.h / Lexer.h / Lexer.cpp       # Lexical analyzer & string translation
│   │   └── Lexer.md                         # [Documentation] Token rules & string handling
│   ├── TreeNode.h / Parser.h / Parser.cpp   # Recursive descent parser & FCNS AST
│   │   └── Parser.md                        # [Documentation] Grammar stack & AST generation
│   ├── Standardizer.h / Standardizer.cpp   # AST simplifications & mapping sugar
│   │   └── Standardizer.md                  # [Documentation] Let, rec, and conditional mappings
│   ├── CSEMachine.h / CSEMachine.cpp       # Execution state engine
│   │   └── CSEMachine.md                    # [Documentation] Machine cycles & primitives
│   └── main.cpp                             # Program entry point & args handler
│       └── main.md                          # [Documentation] Execution flow diagram
├── docs/
│   ├── StepByStepGuide.md                   # Comprehensive step-by-step guide to build it
│   └── ...                                  # Reference PDFs (specifications & grammar)
├── rpal_test_programs/                      # Suite of reference test programs
├── Makefile                                 # Build automation configuration
├── test.sh                                  # Automated differential testing script
└── .gitignore                               # Files excluded from source control
```

---

## Build and Compilation

The project is written in standard C++17 and can be compiled using `make` or any standard C++ compiler.

### Compilation
To compile the interpreter, simply run:
```bash
make
```
This compiles the object files into the `obj/` directory and creates the executable `./rpal20` in the root.

### Cleaning Build Files
To remove object files and compiled binaries:
```bash
make clean
```

---

## How to Run

The executable accepts a single file path to evaluate.

### Execution Mode
Run the interpreter on an RPAL source file:
```bash
./rpal20 rpal_test_programs/add
```

### AST Inspection Mode
You can print the Original and Standardized Abstract Syntax Trees (without executing the program) by supplying the `-ast` flag:
```bash
./rpal20 -ast rpal_test_programs/add
```

---

## Testing & Verification

To verify that the interpreter matches the reference RPAL compiler (`rpal.exe`) behavior, we run differential tests across a suite of programs.

The test programs test lexical constructs, recursive functions (`towers`, `pairs1`), curried applications, tuple manipulations, simultaneous declarations (`and`), and conditional branching.

Run the test suite using:
```bash
bash test.sh
```
The script runs the input programs through both `./rpal.exe` and `./rpal20`, diffs their outputs, and reports pass/fail outcomes. All tests pass with byte-for-byte exact matches.

---

## Educational Resources

If you are looking to build this interpreter yourself, check out the documentation files:
- [Step-by-Step Implementation Guide](docs/StepByStepGuide.md): A full walkthrough of the development stages from Phase 1 to Phase 5.
- [CSE Machine Details](src/CSEMachine.md): Deep dive into instruction flattening, env maps, and resolved debugging edge-cases (tuple unrolling, string concatenation, closure prints).
- [Standardization Rules](src/Standardizer.md): The structural mappings of syntactic constructs.
