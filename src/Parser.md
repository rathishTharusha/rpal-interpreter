# Recursive Descent Parser

The parser takes the sequence of tokens from the Lexer and constructs an Abstract Syntax Tree (AST) representing the structure of the program. It implements the RPAL Phrase Structure Grammar using a recursive descent parser.

## Files
- `src/TreeNode.h`: Defines the `TreeNode` structure. The tree is represented using a binary tree structure: First-Child Next-Sibling (FCNS).
- `src/Parser.h`: Declares the recursive descent parser class interface.
- `src/Parser.cpp`: Contains the implementation of the parser grammar rules, stack operations, and AST serialization.

## Abstract Syntax Tree (AST) Model
A node in the AST is defined by `TreeNode`:
- `type`: The string representing the node type (e.g., `let`, `lambda`, `gamma`, `<IDENTIFIER>`, `<INTEGER>`, etc.).
- `value`: The associated literal value (e.g., variable name or integer value).
- `child`: Pointer to the first child node.
- `sibling`: Pointer to the immediate right sibling of this node.

Using the First-Child Next-Sibling layout, a node can have any number of children. For example, if a node `A` has children `B`, `C`, and `D`, then:
- `A->child` points to `B`.
- `B->sibling` points to `C`.
- `C->sibling` points to `D`.
- `D->sibling` is `nullptr`.

## Phrase Structure Grammar Implementation
The parser implements the RPAL grammar rules using mutual recursion. A stack of AST nodes (`treeStack`) is maintained during parsing:
- **`E()` / `Ew()`**: Expressions and `where` clauses.
- **`T()` / `Ta()` / `Tc()`**: Tuples, list construction (`aug`), and conditional expressions.
- **`B()` / `Bt()` / `Bs()` / `Bp()`**: Boolean expressions and operators (`or`, `&`, `not`, comparison operators).
- **`A()` / `At()` / `Af()` / `Ap()`**: Arithmetic expressions (`+`, `-`, `*`, `/`, `**`).
- **`R()` / `Rn()`**: Evaluator-level expressions and primary terms.
- **`D()` / `Da()` / `Dr()` / `Db()`**: Definitions (let-bindings, simultaneous definitions with `and`, recursive definitions with `rec`).
- **`Vb()` / `Vl()`**: Bound variables and parameter lists.

### Helper Methods
- `read(expectedValue)`: Asserts that the current token has the expected string value, then advances the Lexer.
- `readToken(expectedType)`: Asserts that the current token is of the expected type, creates a leaf node, pushes it onto `treeStack`, and advances.
- `buildTree(type, value, numChildren)`: Pops `numChildren` nodes off the `treeStack` and hooks them up in a first-child next-sibling list under a new parent node of the specified `type` and `value`. Pushes the new parent node back onto the stack.

## What We Did
- Verified key-word checking to prevent identifiers from clashing with RPAL keywords (e.g., `let`, `fn`, `in`, `where`, `rec`, `and`).
- Implemented AST printing using indentation (`.` representing tree depth) to match the reference format when running with the `-ast` flag.
