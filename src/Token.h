#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    IDENTIFIER,
    INTEGER,
    STRING,
    OPERATOR,
    PUNCTUATION,
    END_OF_FILE,
    DELETE_TOKEN, // For whitespace and comments that should be ignored
    UNKNOWN
};

class Token {
public:
    std::string value;
    TokenType type;
    int lineNumber;

    Token(std::string val, TokenType t, int line) : value(val), type(t), lineNumber(line) {}
};

#endif // TOKEN_H
