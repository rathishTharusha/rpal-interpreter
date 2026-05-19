# RPAL_GRAMMAR.md

# Source of Truth: RPAL Phrase Structure Grammar & AST Generation

## Legend

* `->` indicates a parsing rule (what functions to call / tokens to consume).
* `=>` indicates an AST node to build. If `=>` is followed by a string (e.g., `'let'`), build an internal node with that label.

## Grammar Rules

**Expressions**

* `E -> 'let' D 'in' E => 'let'`
* `E -> 'fn' Vb+ '.' E => 'lambda'`
* `E -> Ew;`
* `Ew -> T 'where' Dr => 'where'`
* `Ew -> T;`

**Tuple Expressions**

* `T -> Ta ( ',' Ta )+ => 'tau'`
* `T -> Ta;`
* `Ta -> Ta 'aug' Tc => 'aug'`
* `Ta -> Tc;`
* `Tc -> B '->' Tc '|' Tc => '->'`
* `Tc -> B;`

**Boolean Expressions**

* `B -> B 'or' Bt => 'or'`
* `B -> Bt;`
* `Bt -> Bt '&' Bs => '&'`
* `Bt -> Bs;`
* `Bs -> 'not' Bp => 'not'`
* `Bs -> Bp;`
* `Bp -> A ('gr' | '>') A => 'gr'`
* `Bp -> A ('ge' | '>=') A => 'ge'`
* `Bp -> A ('ls' | '<') A => 'ls'`
* `Bp -> A ('le' | '<=') A => 'le'`
* `Bp -> A 'eq' A => 'eq'`
* `Bp -> A 'ne' A => 'ne'`
* `Bp -> A;`

**Arithmetic Expressions**

* `A -> A '+' At => '+'`
* `A -> A '-' At => '-'`
* `A -> '+' At;`
* `A -> '-' At => 'neg'`
* `A -> At;`
* `At -> At '*' Af => '*'`
* `At -> At '/' Af => '/'`
* `At -> Af;`
* `Af -> Ap '**' Af => '**'`
* `Af -> Ap;`
* `Ap -> Ap '@' '<IDENTIFIER>' R => '@'`
* `Ap -> R;`

**Rators and Rands**

* `R -> R Rn => 'gamma'`
* `R -> Rn;`
* `Rn -> '<IDENTIFIER>'`
* `Rn -> '<INTEGER>'`
* `Rn -> '<STRING>'`
* `Rn -> 'true' => 'true'`
* `Rn -> 'false' => 'false'`
* `Rn -> 'nil' => 'nil'`
* `Rn -> '(' E ')'`
* `Rn -> 'dummy' => 'dummy'`

**Definitions**

* `D -> Da 'within' D => 'within'`
* `D -> Da;`
* `Da -> Dr ( 'and' Dr )+ => 'and'`
* `Da -> Dr;`
* `Dr -> 'rec' Db => 'rec'`
* `Dr -> Db;`
* `Db -> Vl '=' E => '='`
* `Db -> '<IDENTIFIER>' Vb+ '=' E => 'fcn_form'`
* `Db -> '(' D ')';`

**Variables**

* `Vb -> '<IDENTIFIER>'`
* `Vb -> '(' Vl ')'`
* `Vb -> '(' ')' => '()'`
* `Vl -> '<IDENTIFIER>' list ',' => ','?`
