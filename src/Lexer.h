#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <vector>
#include "Token.h"

class Lexer {
private:
    std::ifstream file;
    char currentChar;
    int currentLine;

    void advance();
    char peek();
    
    bool isLetter(char c);
    bool isDigit(char c);
    bool isOperatorSymbol(char c);
    bool isPunctuation(char c);
    bool isWhitespace(char c);

    Token getNextRealToken();

public:
    Lexer(const std::string& filename);
    ~Lexer();

    Token getNextToken();
    std::vector<Token> getAllTokens(); // useful for testing
};

#endif // LEXER_H
