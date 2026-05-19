# RPAL_LEXICON.md

# Source of Truth: RPAL Lexical Analyzer Rules

## Token Definitions

The Lexer must read the input character stream and produce tokens based strictly on the following Regular Expressions.
*Note: `->` denotes the grammar rule to match, and `=>` denotes the Token Type to output.*

1. **Identifiers**
    * Rule: `Identifier -> Letter (Letter | Digit | '_')*`
    * Output: `=> '<IDENTIFIER>'`
2. **Integers**
    * Rule: `Integer -> Digit+`
    * Output: `=> '<INTEGER>'`
3. **Operators**
    * Rule: `Operator -> Operator_symbol+`
    * Output: `=> '<OPERATOR>'`
4. **Strings**
    * Rule: `String -> '\'' (Letter | Digit | Operator_symbol | Spaces | Punctuation)* '\''`
    * Output: `=> '<STRING>'`
    * *Note: Strings are enclosed in single quotes. You must handle escape sequences if required by standard RPAL test cases.*
5. **Spaces (Whitespace)**
    * Rule: `Spaces -> (' ' | Horizontal_Tab | End_of_Line)+`
    * Output: `=> '<DELETE>'` (The lexer must consume and ignore these).
6. **Comments**
    * Rule: `Comment -> '//' (Letter | Digit | Operator_symbol | Spaces | Punctuation)* End_of_Line`
    * Output: `=> '<DELETE>'` (The lexer must consume and ignore these).
7. **Punctuation**
    * Rules:
        * `-> '('  => '('`
        * `-> ')'  => ')'`
        * `-> ';'  => ';'`
        * `-> ','  => ','`

## Character Sets

* **Letter:** `'A'..'Z' | 'a'..'z'`
* **Digit:** `'0'..'9'`
* **Operator_symbol:** `'+' | '-' | '*' | '<' | '>' | '&' | '.' | '@' | '/' | ':' | '=' | '~' | '|' | '$' | '!' | '#' | '%' | '^' | '_' | '[' | ']' | '{' | '}' | '"' | '\`' | '?'`
