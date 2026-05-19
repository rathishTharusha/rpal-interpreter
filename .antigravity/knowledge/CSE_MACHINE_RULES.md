# CSE_MACHINE_RULES.md

# Source of Truth: The CSE Machine Evaluator (Operational Semantics)

## 1. Machine Architecture

The CSE machine consists of three strictly managed data structures:

* **C (Control):** A stack-like structure representing the sequence of operations. It is initialized with the sequence `[ e0, PE ]` where `e0` is the flattened Standardized Tree and `PE` is the Primitive Environment. Items are popped from the right (top).
* **S (Stack):** A stack holding operational values (integers, strings, booleans, tuples, closures).
* **E (Environment):** A hierarchical tree of dictionaries mapping variable names to their bound values. Every environment (except PE) has a parent environment pointer.

## 2. Supported Data Types on the Stack (S)

* **Primitive Types:** Integer, String, TruthValue (`true`/`false`), Dummy (`dummy`), Nil (`nil`).
* **Closures:** Encapsulates a lambda function and its environment pointer: `[lambda(X, E), env_pointer]`.
* **Eta-Closures:** Used for recursion (Y*). Encapsulates a standard closure: `eta[closure]`.
* **Tuples:** A dynamically sized array of any supported data types.

## 3. The 13 Execution Rules

The execution engine runs a `while` loop until **Control (C)** is empty. At each tick, the engine inspects the top item on C and applies the matching rule:

* **Rule 1: Name Lookup**
  * **Condition:** Top of C is an `<IDENTIFIER>` (e.g., `x`).
  * **Action:** Pop `x` from C. Look up the value bound to `x` starting in the current environment and traversing up to the parent environments until found. Push the resolved value onto S.

* **Rule 2: Constants**
  * **Condition:** Top of C is a primitive constant (`<INTEGER>`, `<STRING>`, `true`, `false`, `dummy`, `nil`).
  * **Action:** Pop the constant from C and push it directly onto S.

* **Rule 3: Lambda (Function Definition)**
  * **Condition:** Top of C is a `lambda X.E` node.
  * **Action:** Pop the lambda from C. Create a closure `[lambda X.E, current_env]` binding it to the active environment. Push this closure onto S.

* **Rule 4: Gamma (Function Application)**
  * **Condition:** Top of C is `gamma`. The top of S is a closure `[lambda X.E, env_c]`, and the next item on S is a value `V`.
  * **Action:** 1. Pop `gamma` from C.
    2. Pop the closure and value `V` from S.
    3. Create a new environment `env_new` whose parent is `env_c`.
    4. Bind the variable `X` to the value `V` inside `env_new`.
    5. Push the *current* environment marker onto C (to restore it later).
    6. Push the lambda body `E` onto C.
    7. Set the current environment pointer to `env_new`.

* **Rule 5: Environment Restore**
  * **Condition:** Top of C is an environment marker `env_n`.
  * **Action:** Pop `env_n` from C. Set the current environment pointer to `env_n`. (Leave S untouched).

* **Rule 6: Binary Operators**
  * **Condition:** Top of C is a binary operator (`+`, `-`, `*`, `/`, `**`, `eq`, `ne`, `gr`, `ge`, `ls`, `le`, `or`, `&`).
  * **Action:** Pop the operator. Pop `rand1` and `rand2` from S. Compute the result. Push the result onto S.

* **Rule 7: Unary Operators**
  * **Condition:** Top of C is a unary operator (`neg`, `not`).
  * **Action:** Pop the operator. Pop `rand1` from S. Compute the result. Push the result onto S.

* **Rule 8: Beta (Conditional Execution)**
  * **Condition:** Top of C is `beta`. The top of S is a TruthValue `B`.
  * **Action:** Pop `beta` from C. Pop `B` from S. The next two items on C will be control structures `delta_then` and `delta_else`.
    * If `B` is `true`: Pop both deltas from C, push `delta_then` onto C.
    * If `B` is `false`: Pop both deltas from C, push `delta_else` onto C.

* **Rule 9: Tau (Tuple Formation)**
  * **Condition:** Top of C is `tau_n` (where `n` is an integer).
  * **Action:** Pop `tau_n` from C. Pop `n` items from S. Combine them into an `n`-ary tuple and push the tuple back onto S.

* **Rule 10: Tuple Selection**
  * **Condition:** Top of C is `gamma`. Top of S is a Tuple `T`, and the next item on S is an Integer `i`.
  * **Action:** Pop `gamma` from C. Pop `T` and `i` from S. Push the `i`-th element of `T` (1-indexed) onto S.

* **Rule 11: Y* (Recursion Initialization)**
  * **Condition:** Top of C is `gamma`. Top of S is the primitive `Y*`, and the next item on S is a closure `f`.
  * **Action:** Pop `gamma` from C. Pop `Y*` and `f` from S. Wrap the closure into an eta-closure: `eta[f]`. Push `eta[f]` onto S.

* **Rule 12: Eta Application (Recursive Call)**
  * **Condition:** Top of C is `gamma`. Top of S is an eta-closure `eta[f]`, and the next item on S is a value `V`.
  * **Action:** 1. Pop `gamma` from C.
    2. Pop `eta[f]` and `V` from S.
    3. Push two `gamma` nodes onto C.
    4. Push `eta[f]`, the raw closure `f`, and the value `V` onto S (in that order).

* **Rule 13: Standard Primitives**
  * **Condition:** Top of C is `gamma`. Top of S is a built-in primitive function (e.g., `Print`, `Order`, `Isinteger`, `Istuple`, `Conc`, `Stem`, `Stern`, `ItoS`).
  * **Action:** Pop `gamma` from C. Pop the primitive and its required arguments from S. Execute the built-in C++ logic (e.g., print to `stdout`, check type). Push the resulting value onto S (if applicable).
