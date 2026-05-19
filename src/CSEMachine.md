# Control Stack Evaluator (CSE) Machine

The CSE Machine is the execution engine of the interpreter. It is a state machine that evaluates the standardized syntax tree by manipulating a **Control Stack**, a **Value Stack**, and a tree of **Environments**.

## Files
- `src/CSEMachine.h`: Defines the `CSEItem`, `Environment`, and `CSEMachine` class structures.
- `src/CSEMachine.cpp`: Implements the evaluation loop, environment bindings, primitive function application, and binary/unary operators.

## Machine Architecture

### 1. Control Stack (`control`)
Contains instructions to be evaluated. It is initialized by flattening the standardized AST. During evaluation, instructions are popped off the Control Stack.
- High-level nodes (like lambda closures and conditionals) push new blocks of instructions (Deltas) onto the Control Stack.

### 2. Value Stack (`stack`)
Contains temporary results, closures, primitive functions, environments, and evaluated values (integers, strings, truth values, tuples).

### 3. Environment Tree
Maintains the bindings of identifiers to values.
- `E0` is the global environment containing built-in primitives.
- When a user-defined function (closure) is applied, a new environment `Ei` is created. Its parent pointer links back to the environment where the closure was *defined* (lexical scoping).

---

## Execution Mechanics

### 1. AST Flattening
The tree is flattened into a collection of sequential blocks called **Deltas** (`deltas[i]`).
- `Delta 0` represents the root expression.
- Every `lambda` node creates a new child Delta containing the instructions representing the function body.
- The tree-walking method `flatten()` traverses the tree and populates `deltas`, inserting `LAMBDA` markers that reference their respective Delta indices.

### 2. The Main Evaluation Loop
The loop runs until the `control` stack is empty:
- **Value Push**: Constants like `INTEGER`, `STRING`, `TRUTH_VALUE`, `DUMMY`, `NIL` are pushed directly to the Value Stack.
- **Identifier Lookup**: Identifiers are resolved by searching the current environment `current_env` and climbing up parent environments until found.
- **LAMBDA**: Creates a `CLOSURE` containing the bound variable(s), the body's Delta index, and a pointer to `current_env` (the definition environment).
- **GAMMA (Application)**: Applies the operator on the top of the stack (`rator`) to the operand (`rand`).
  - **Closure Application**: Creates a new environment `En`, binds the parameters, sets `current_env` to `En`, and pushes the body's Delta onto the control stack along with an `ENV_MARKER` to restore the previous environment.
  - **Primitive Application**: Executes built-in functions directly.
  - **Tuple Selection**: If `rator` is a tuple and `rand` is an integer, it extracts the `rand`-th item (1-indexed) from the tuple.
- **Environment Marker (`ENV_MARKER`)**: Restores `current_env` to the caller's environment.
- **Beta / Conditionals**: If a conditional branch (`beta`) is encountered, it evaluates the boolean test on the stack and pushes the instructions from either the `then` or `else` Delta.

---

## Primitive Operators Supported

| Primitive | Action |
|---|---|
| `Print` | Prints the value representation to stdout (with string escape characters handled). |
| `Stem` | Returns the first character of a string. |
| `Stern` | Returns the string excluding the first character. |
| `Conc` | Concatenates two strings, or concatenates the elements of a tuple into a single string. |
| `Order` | Returns the number of items in a tuple. |
| `Null` | Returns `true` if the operand is `nil` or an empty tuple. |
| `Isinteger` / `Isstring` / `Istuple` / `Istruthvalue` / `Isfunction` | Dynamic type-assertion checks. |
| `ItoS` | Converts an integer value to a string value. |
| `Cond` | Evaluates conditional logic. |
| `aug` | Appends a value to a tuple (or creates a tuple if appended to `nil`). |
| `Ystar` | Evaluates fixed-point Combinator for recursion. |

---

## Critical Bug Fixes and Details

### 1. `aug` Tuple Construction Semantics
The primitive operator `aug` acts to append an element to a tuple. During testing, recursive list manipulations like `pairs1` would result in a stack underflow. This was because if `rand2` was a tuple, `aug` was unrolling the tuple items instead of keeping the tuple intact and appending the item:
- **Fixed**: `aug` now correctly checks the type and adds the element directly to the end of the tuple structure without destructive unrolling.

### 2. `tau` Standardization Conflict
The `tau` operator collects a defined number of stack values into a single tuple.
- **Problem**: When a standardized tree contained `tau` nodes (e.g., from simultaneous definitions), the machine would either crash or skip evaluation.
- **Fixed**: We added `tau` instruction mapping during compilation and updated the evaluation loop to intercept `tau`, pop exactly `n` arguments from the value stack, construct a `TUPLE` node, and push it back.

### 3. `Conc` Tuple and String Semantics
The `Conc` operator has dual behaviors depending on standard currying vs tuple application.
- **Problem**: Running `conc.1` produced no output due to mismatched concatenation types.
- **Fixed**: Updated `Conc` to check if its argument is a tuple. If it is a tuple, it performs a full string concatenation of all string elements within that tuple. If it is a single string, it returns a partially-applied `Conc1` operator awaiting the second operand.

### 4. Environment Formatting vs. Delta Indexes
- **Problem**: When printing closures, the expected output was `[lambda closure: x: 2]` but the interpreter output `[lambda closure: x: 0]`.
- **Fixed**: Discovered that the reference interpreter tracks the **Delta Index** (representing the static scope body) of the lambda in its print representation, not the runtime environment ID. Updating the print output format resolved the discrepancy.
