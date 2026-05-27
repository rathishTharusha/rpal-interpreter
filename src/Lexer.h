#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <vector>
#include "Token.h"

/**
 * @brief Lexical analyzer for the RPAL language.
 * 
 * Reads characters from an RPAL source file, filters comments and
 * whitespace (through a Screener phase), and generates a stream of Tokens.
 */
class Lexer {
private:
    std::ifstream file;   // Input file stream
    char currentChar;      // The current lookahead character
    int currentLine;       // Tracks current line number for error reporting

    // Reads the next character from the file stream and updates state
    void advance();
    
    // Looks at the next character in the stream without consuming it
    char peek();
    
    // Character classification helpers
    bool isLetter(char c);
    bool isDigit(char c);
    bool isOperatorSymbol(char c);
    bool isPunctuation(char c);
    bool isWhitespace(char c);

    // Core scanner function extracting the next raw token (whitespace/comments included)
    Token getNextRealToken();

public:
    // Initializes the lexer with the given file path
    Lexer(const std::string& filename);
    ~Lexer();

    // Returns the next token in the stream, automatically filtering out whitespace and comments
    Token getNextToken();
    
    // Helper to extract all tokens in one pass (useful for verification)
    std::vector<Token> getAllTokens();
};

#endif // LEXER_H
