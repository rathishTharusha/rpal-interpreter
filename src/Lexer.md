# Lexical Analyzer (Lexer)

The lexical analyzer is responsible for transforming a raw character stream from an RPAL source file into a sequence of tokens. It discards comments and whitespace, and flags unknown characters.

## Files
- `src/Token.h`: Defines the `TokenType` enumeration and the `Token` class structure.
- `src/Lexer.h`: Declares the `Lexer` class interface.
- `src/Lexer.cpp`: Contains the implementation of the lexer logic.

## Token Types
Defined in `src/Token.h`:
- `IDENTIFIER`: Variables, function names, and built-in names (starts with a letter, followed by letters, digits, or underscores).
- `INTEGER`: Numeric values (sequence of digits).
- `STRING`: String literals wrapped in single quotes `'...'`. It supports escape sequences like `\t`, `\n`, `\\`, and `\'`.
- `OPERATOR`: Mathematical, logical, and structural operator symbols (e.g., `+`, `-`, `*`, `/`, `&`, `or`, `eq`, `ge`, etc.).
- `PUNCTUATION`: Separators and delimiters like `(`, `)`, `;`, `,`.
- `DELETE_TOKEN`: Used internally to flag whitespace and comments so they can be filtered out during processing.
- `END_OF_FILE`: Marker for reaching the end of the source file.

## Implementation Details

### Lexical Rules
The Lexer classifies tokens using helper methods:
- `isLetter(char c)`: Checks if a character is `[a-zA-Z]`.
- `isDigit(char c)`: Checks if a character is `[0-9]`.
- `isOperatorSymbol(char c)`: Checks if a character is one of: `+`, `-`, `*`, `<`, `>`, `&`, `.`, `@`, `/`, `:`, `=`, `~`, `|`, `$`, `!`, `#`, `%`, `^`, `_`, `[`, `]`, `{`, `}`, `"`, `` ` ``.
- `isPunctuation(char c)`: Checks if a character is `(`, `)`, `;`, or `,`.
- `isWhitespace(char c)`: Checks if a character is a space, tab, or carriage return/newline.

### Tokenization Flow
1. **Source Loading**: Opens the source file as an `std::ifstream` and reads character-by-character.
2. **Whitespace and Comments**: Skips whitespace and line/block comments. Line comments in RPAL start with `//` and go to the end of the line.
3. **Identifier Extraction**: Reads a contiguous sequence of letters, digits, and underscores.
4. **Integer Extraction**: Reads a contiguous sequence of digits.
5. **String Literal Extraction**: Reads characters enclosed in `'` single quotes. If a backslash `\` is encountered, it processes escape characters (`\n`, `\t`, `\\`, `\'`) to build the final string value.
6. **Operator Extraction**: Continues reading characters matching the operator symbol list to build multi-character operator names (e.g., `->`, `**`, `>=`, `//`).
7. **Punctuation Extraction**: Reads single punctuation symbols.

## What We Did
- Handled escape character translation within string literals so they map correctly to standard C++ string escape characters (essential for correctness of output formatting).
- Provided robust line number tracking to report accurate syntax errors.
