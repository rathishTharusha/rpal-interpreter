# AST Standardizer

The Standardizer modifies the parsed Abstract Syntax Tree (AST) into a standard, simplified form before it is evaluated by the CSE Machine. This step maps higher-level syntactic sugar (such as nested function declarations, let-bindings, and simultaneous declarations) into core lambda-calculus representations.

## Files
- `src/Standardizer.h`: Declares the `Standardizer` class.
- `src/Standardizer.cpp`: Implements the tree-walking transformation rules.

## Standardization Rules
The Standardizer performs a post-order traversal (standardizing children first) and maps nodes as follows:

| AST Construct | Original Node Structure | Standardized Structure | Explanation |
|---|---|---|---|
| **Let-Binding** | `let ( = X E ) P` | `gamma ( lambda X P ) E` | Invokes the body `P` as a lambda applied to the initialization value `E`. |
| **Where-Clause** | `where P ( = X E )` | `gamma ( lambda X P ) E` | Identical semantics to let-binding. |
| **Function Definition Form** | `fcn_form f V1 V2 ... Vn E` | `(= f (lambda V1 (lambda V2 ... (lambda Vn E))))` | Translates n-ary parameters into a curried function definition. |
| **Simultaneous Definition** | `and (= X1 E1) (= X2 E2) ...` | `(= ( , X1 X2 ... ) ( tau E1 E2 ... ))` | Combines identifiers into a comma-list and values into a tuple. |
| **Recursive Definition** | `rec (= X E)` | `(= X (gamma Ystar (lambda X E)))` | Wraps the definition with the fixed-point combinator `Ystar` to support recursion. |
| **Infix Operator Application** | `@ E1 N E2` | `gamma ( gamma N E1 ) E2` | Curries the binary operation `N` applied to `E1` and `E2`. |
| **Conditional Expression** | `-> B T E` | `gamma ( gamma ( gamma Cond B ) (lambda () T) ) (lambda () E)` | Evaluates conditional branching dynamically by wrapping then/else blocks in dummy lambdas. |
| **N-ary Lambdas** | `lambda V1 V2 ... Vn E` | `lambda V1 (lambda V2 ... (lambda Vn E))` | Translates multi-variable lambdas into curried single-variable lambdas. |

## Parameter Tupling vs Currying
If a lambda has a variable list defined inside parentheses (e.g., `lambda (x, y). E`), it indicates parameter tupling. The Standardizer handles this by introducing a temporary variable `T++` and mapping it to:
`lambda T++. (gamma (lambda x. (gamma (lambda y. E) (gamma T++ 2))) (gamma T++ 1))`
This pulls elements out of the tuple `T++` by indexing them during parameter binding.

## What We Did
- Resolved standardizer conflicts by removing manual overrides for boolean `not` and unary `neg` (which are evaluated as native operators by the CSE machine instead of standardized terms).
- Fixed the tree restructuring logic to ensure sibling nodes are correctly re-linked when a node is replaced by its standardized version.
