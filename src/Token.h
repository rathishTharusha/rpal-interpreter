#ifndef TOKEN_H
#define TOKEN_H

#include <string>

/**
 * @brief Token categories used by the RPAL Lexer and Parser.
 */
enum class TokenType {
    IDENTIFIER,   // Variable names, keywords (let, in, fn, etc.)
    INTEGER,      // Numeric literals
    STRING,       // Single-quoted string literals ('...')
    OPERATOR,     // Math and logic symbols (+, -, *, aug, etc.)
    PUNCTUATION,  // Syntax markers like parentheses, semicolons, and commas
    END_OF_FILE,  // Special marker representing end of input
    DELETE_TOKEN, // Internal token for spaces, tabs, newlines, and comments
    UNKNOWN       // Error fallback token for invalid characters
};

/**
 * @brief Represents a lexical token in the source program.
 */
class Token {
public:
    std::string value;    // The actual lexeme text
    TokenType type;       // Categorized token type
    int lineNumber;       // The line number where this token occurred (1-indexed)

    Token(std::string val, TokenType t, int line) : value(val), type(t), lineNumber(line) {}
};

#endif // TOKEN_H
