# Step-by-Step Guide to Building an RPAL Interpreter from Scratch

This guide is designed to walk you through the implementation of an interpreter for the **Recursive Program Association Language (RPAL)** from scratch, using C++. It is structured around the five logical phases of language development and incorporates the lessons and debugging insights from our reference implementation.

---

## Phase 1: Lexical Analysis (Lexer)

The Lexer scans the input file character-by-character and groups them into logical tokens.

### Key Steps:
1. **Define Token Structures**: Create a `Token` class storing the token's type, string value, and line number.
2. **Implement Character Classification**: Create predicates for checking letters, digits, punctuation, and operator symbols.
3. **Skipping Noise**: Ignore spaces, tabs, newlines, and comments (`//...` and `/*...*/`).
4. **Token Assembly**:
   - String literals enclosed in single quotes `'...'` must support character escape processing (such as converting a literal `\t` or `\n` sequence into their corresponding control characters).
   - Operators can be multi-character (e.g., `->`, `**`, `//`, `:=`). Scan greedily.
5. **Output**: Expose a `getNextToken()` interface.

> [!TIP]
> Ensure you track line numbers accurately to make parsing errors easy to locate.

---

## Phase 2: AST & Parsing

The Parser consumes tokens and builds an Abstract Syntax Tree (AST) using a First-Child Next-Sibling (FCNS) binary tree structure.

### Key Steps:
1. **Define FCNS Tree Representation**:
   ```cpp
   class TreeNode {
   public:
       std::string type;
       std::string value;
       std::shared_ptr<TreeNode> child;
       std::shared_ptr<TreeNode> sibling;
   };
   ```
2. **Implement Phrase Structure Grammar**: Convert the RPAL grammar rules into a set of recursive methods (`E()`, `T()`, `B()`, `A()`, `D()`, etc.).
3. **Manage the Tree Stack**: Use a stack of `TreeNode` pointers to collect parsed leaves and build tree structures:
   - When a rule matches `n` sub-expressions, pop them from the stack, hook them up as sibling chains, and attach them under a new parent node.
4. **Print Utility**: Print the AST using `.` characters to represent tree depth (pre-order traversal).

---

## Phase 3: AST Standardization

Standardization simplifies the AST by converting complex language features into core lambda-calculus structures (`gamma`, `lambda`, `tau`, etc.).

### Key Steps:
1. **Walk the Tree**: Traverse the tree post-order (bottom-up) to standardize child nodes before transforming parent nodes.
2. **Apply Transformation Rules**:
   - `let X = E in P` $\rightarrow$ `gamma (lambda X. P) E`
   - `fcn_form f V1..Vn = E` $\rightarrow$ curried lambdas.
   - `rec (= X E)` $\rightarrow$ Y-Combinator wrapping: `X = Ystar (lambda X. E)`.
   - `B -> T | E` $\rightarrow$ `gamma(gamma(gamma(Cond, B), lambda (). T), lambda (). E)`.
3. **Handle Multi-Parameter Tupling**: If a lambda takes a comma-list of variables, e.g. `lambda (x, y). E`, translate it by binding a temporary tuple parameter `T++` and resolving indexes dynamically.

---

## Phase 4: The CSE Machine Foundation

The Control Stack Evaluator (CSE) Machine executes the standardized tree.

### Key Steps:
1. **Define the Engine's Components**:
   - **Control Stack**: Pushes instructions to evaluate.
   - **Value Stack**: Stores evaluated constants, tuples, and closures.
   - **Environment**: Holds identifier bindings and links to a parent environment.
2. **Flatten the AST**:
   - Traverse the standardized AST to partition it into sequential instruction blocks (Deltas).
   - Any `lambda` node creates a new child Delta. Insert a `LAMBDA(delta_index)` marker in the parent Delta in place of the lambda body.
3. **Implement the Loop**:
   - Pop instructions from the Control Stack.
   - Look up variables in the active environment.
   - Push primitives and closures onto the Value Stack.
   - Maintain scope entries/exits via `ENV_MARKER` values on the control stack.

---

## Phase 5: Primitives & Final Parity Debugging

This phase brings the interpreter to 100% correctness by implementing primitive behaviors and debugging corner cases.

### Crucial Implementation details:
1. **Tuple Indexing & Appending (`aug`)**:
   - The `aug` primitive merges an item into a tuple. Make sure that if the second argument (`rand2`) is already a tuple, `aug` appends it as an element rather than unrolling it.
2. **Direct `tau` Evaluation**:
   - Standardizing simultaneous definitions generates a `tau` node. In the CSE machine, intercept the `tau` node, pop `n` items from the stack, package them into a `TUPLE` value, and push it back to the value stack.
3. **String Concatenation (`Conc`)**:
   - Handle dual uses: `Conc x y` should partially apply `Conc1(x)` then combine with `y`. If `Conc` receives a single `TUPLE` containing strings, it should concatenate the whole tuple into a single string immediately.
4. **Environment ID vs Delta Index in Closures**:
   - When printing a closure, the format is `[lambda closure: <var>: <idx>]`. Note that `<idx>` refers to the **Delta Index** of the lambda's body, not the runtime environment ID.

---

## Verification
You can verify your implementation by running programs and checking their stdout output against the reference `rpal.exe` binary. 
Ensure you compile with C++17 support:
```bash
g++ -std=c++17 src/*.cpp -o rpal20
```
Use the provided `test.sh` script to automate checking all test programs under `rpal_test_programs/` for byte-by-byte parity.
