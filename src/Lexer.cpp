#include "Lexer.h"
#include <iostream>
#include <stdexcept>

Lexer::Lexer(const std::string& filename) : currentLine(1) {
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    advance();
}

Lexer::~Lexer() {
    if (file.is_open()) {
        file.close();
    }
}

void Lexer::advance() {
    if (file.get(currentChar)) {
        if (currentChar == '\n') {
            currentLine++;
        }
    } else {
        currentChar = EOF;
    }
}

char Lexer::peek() {
    return file.peek();
}

bool Lexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isDigit(char c) {
    return (c >= '0' && c <= '9');
}

bool Lexer::isOperatorSymbol(char c) {
    const std::string ops = "+-*<>&.@/:=~|$!#%^_[}\"{`?";
    return ops.find(c) != std::string::npos;
}

bool Lexer::isPunctuation(char c) {
    return c == '(' || c == ')' || c == ';' || c == ',';
}

bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

Token Lexer::getNextToken() {
    Token token = getNextRealToken();
    while (token.type == TokenType::DELETE_TOKEN) {
        token = getNextRealToken();
    }
    return token;
}

Token Lexer::getNextRealToken() {
    if (currentChar == EOF || file.eof()) {
        return Token("", TokenType::END_OF_FILE, currentLine);
    }

    if (isWhitespace(currentChar)) {
        std::string val = "";
        while (currentChar != EOF && isWhitespace(currentChar)) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::DELETE_TOKEN, currentLine);
    }

    if (currentChar == '/' && peek() == '/') {
        // Comment
        std::string val = "";
        while (currentChar != EOF && currentChar != '\n') {
            val += currentChar;
            advance();
        }
        if (currentChar == '\n') {
            val += currentChar;
            advance(); // consume newline
        }
        return Token(val, TokenType::DELETE_TOKEN, currentLine);
    }

    if (isLetter(currentChar)) {
        std::string val = "";
        int startLine = currentLine;
        while (currentChar != EOF && (isLetter(currentChar) || isDigit(currentChar) || currentChar == '_')) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::IDENTIFIER, startLine);
    }

    if (isDigit(currentChar)) {
        std::string val = "";
        int startLine = currentLine;
        while (currentChar != EOF && isDigit(currentChar)) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::INTEGER, startLine);
    }

    if (currentChar == '\'') {
        std::string val = "";
        int startLine = currentLine;
        advance(); // consume opening quote
        while (currentChar != EOF && currentChar != '\'') {
            if (currentChar == '\\') {
                advance();
                if (currentChar == 't') val += '\t';
                else if (currentChar == 'n') val += '\n';
                else if (currentChar == '\\') val += '\\';
                else if (currentChar == '\'') val += '\'';
                else val += currentChar;
            } else {
                val += currentChar;
            }
            advance();
        }
        if (currentChar == '\'') {
            advance(); // consume closing quote
        }
        return Token(val, TokenType::STRING, startLine);
    }

    if (isOperatorSymbol(currentChar)) {
        std::string val = "";
        int startLine = currentLine;
        while (currentChar != EOF && isOperatorSymbol(currentChar)) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::OPERATOR, startLine);
    }

    if (isPunctuation(currentChar)) {
        std::string val = std::string(1, currentChar);
        int startLine = currentLine;
        advance();
        return Token(val, TokenType::PUNCTUATION, startLine);
    }

    std::string val = std::string(1, currentChar);
    int startLine = currentLine;
    advance();
    return Token(val, TokenType::UNKNOWN, startLine);
}

std::vector<Token> Lexer::getAllTokens() {
    std::vector<Token> tokens;
    Token t = getNextToken();
    while (t.type != TokenType::END_OF_FILE) {
        tokens.push_back(t);
        t = getNextToken();
    }
    tokens.push_back(t);
    return tokens;
}
