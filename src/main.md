# Entry Point (main)

The `src/main.cpp` file contains the entry point for the RPAL interpreter. It handles command line argument parsing, coordinates parsing/standardizing/evaluation, and manages exception handling.

## Files
- `src/main.cpp`: Orchestrates the interpreter pipeline.

## Execution Pipeline Flow

```mermaid
graph TD
    A[Command Line Arguments] --> B{Check -ast Flag}
    B -->|Yes| C[Enable AST Printing]
    B -->|No| D[Run Interpreter Mode]
    C --> E[Parse Source File]
    D --> E
    E --> F[Generate AST]
    F --> G{Is AST Printing Enabled?}
    G -->|Yes| H[Print Original AST]
    G -->|No| I[Standardize AST]
    H --> I
    I --> J{Is AST Printing Enabled?}
    J -->|Yes| K[Print Standardized AST & Exit]
    J -->|No| L[CSE Machine Evaluation]
    L --> M[Print Execution Results]
```

## CLI Configuration
The interpreter accepts a single source file path and an optional `-ast` flag:
- **`./rpal20 [-ast] <filename>`**
  - `<filename>`: Path to the RPAL source file to evaluate.
  - `-ast`: Prints both the Original AST and the Standardized AST to `stdout` instead of evaluating the program.

## Error Handling
The program wraps all stages in a comprehensive `try-catch` block:
- Syntax errors raised during parsing (e.g., mismatched brackets, unexpected tokens) are caught and reported.
- Runtime errors raised during CSE Machine execution (e.g., division by zero, stack underflows, type assertion failures, undeclared identifier lookups) are printed to `stderr` with a non-zero exit code.
