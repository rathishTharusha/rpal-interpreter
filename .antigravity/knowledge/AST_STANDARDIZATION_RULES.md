# AST_STANDARDIZATION_RULES.md

# Source of Truth: Sub-Tree Transformational Grammar

The Standardizer module must recursively traverse the AST. When it identifies a specific tree pattern on the left-hand side, it must mutate the tree into the pattern on the right-hand side. All internal nodes in the final Standardized Tree (ST) must exclusively be `gamma` or `lambda` nodes.

## Rules (Desugaring)

*Note: `(Node Child1 Child2)` represents a sub-tree.*

1. **let**
    * Before: `('let' ('=' X E) P)`
    * After:  `('gamma' ('lambda' X P) E)`
2. **where**
    * Before: `('where' P ('=' X E))`
    * After:  `('gamma' ('lambda' X P) E)`
3. **tau**
    * Before: `('tau' E1 E2 ... En)`
    * After:  `('gamma' ('gamma' 'aug' <standardized tau E1..En-1>) En)` -> *Note: Evaluate `tau` recursively to build augmented tuples with `nil`.*
4. **-> (Conditional)**
    * Before: `('->' B T E)`
    * After:  `('gamma' ('gamma' ('gamma' 'Cond' B) ('lambda' '()' T)) ('lambda' '()' E))`
5. **not**
    * Before: `('not' E)`
    * After:  `('gamma' 'not' E)`
6. **neg**
    * Before: `('neg' E)`
    * After:  `('gamma' 'neg' E)`
7. **within**
    * Before: `('within' ('=' X1 E1) ('=' X2 E2))`
    * After:  `('=' X2 ('gamma' ('lambda' X1 E2) E1))`
8. **rec**
    * Before: `('rec' ('=' X E))`
    * After:  `('=' X ('gamma' 'Ystar' ('lambda' X E)))`
9. **fcn_form**
    * Before: `('fcn_form' P V E)`
    * After:  `('=' P ('lambda' V E))` -> *Note: For multiple variables `V+`, chain lambdas.*
10. **lambda (multi-variable)**
    * Before: `('lambda' (',' V1 V2 ... Vn) E)`
    * After:  Nested lambda extraction mapping to `X.i` components.
11. **and**
    * Before: `('and' ('=' X1 E1) ... ('=' Xn En))`
    * After:  `('=' (',' X1 ... Xn) ('tau' E1 ... En))`
12. **@ (Infix Operator)**
    * Before: `('@' E1 N E2)`
    * After:  `('gamma' ('gamma' N E1) E2)`
