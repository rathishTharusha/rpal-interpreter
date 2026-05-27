#include "Lexer.h"
#include <iostream>
#include <stdexcept>

/**
 * @brief Constructor. Opens the source file and prepares the first character.
 * @param filename Path to the RPAL source file.
 */
Lexer::Lexer(const std::string& filename) : currentLine(1) {
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    advance(); // Load the first character into currentChar
}

/**
 * @brief Destructor. Closes the file stream if open.
 */
Lexer::~Lexer() {
    if (file.is_open()) {
        file.close();
    }
}

/**
 * @brief Consumes the current character and reads the next one from the file stream.
 * Increments the line counter upon encountering a newline.
 */
void Lexer::advance() {
    if (file.get(currentChar)) {
        if (currentChar == '\n') {
            currentLine++;
        }
    } else {
        currentChar = EOF; // Mark end of file
    }
}

/**
 * @brief Returns the next character in the stream without consuming it.
 * @return The next character, or EOF if at the end of the file.
 */
char Lexer::peek() {
    return file.peek();
}

/**
 * @brief Checks if a character is an alphabetical letter.
 */
bool Lexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * @brief Checks if a character is a decimal digit.
 */
bool Lexer::isDigit(char c) {
    return (c >= '0' && c <= '9');
}

/**
 * @brief Checks if a character belongs to the set of operators defined in the RPAL lexicon.
 */
bool Lexer::isOperatorSymbol(char c) {
    const std::string ops = "+-*<>&.@/:=~|$!#%^_[}\"{`?";
    return ops.find(c) != std::string::npos;
}

/**
 * @brief Checks if a character is standard syntax punctuation.
 */
bool Lexer::isPunctuation(char c) {
    return c == '(' || c == ')' || c == ';' || c == ',';
}

/**
 * @brief Checks if a character is a whitespace character (space, tab, or newline).
 */
bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 * @brief High-level scanner entry point.
 * Iterates through raw tokens, filtering out comments and whitespace (Screener).
 * @return The next real, syntax-relevant Token.
 */
Token Lexer::getNextToken() {
    Token token = getNextRealToken();
    while (token.type == TokenType::DELETE_TOKEN) {
        token = getNextRealToken();
    }
    return token;
}

/**
 * @brief Core scanning logic. Matches character classes to form lexemes.
 * Implements greedy multi-character operator matching, string literal escape
 * sequences, and single/multi-line comment skipping.
 * @return A Token containing the lexeme, its class, and line number.
 */
Token Lexer::getNextRealToken() {
    if (currentChar == EOF || file.eof()) {
        return Token("", TokenType::END_OF_FILE, currentLine);
    }

    // 1. Whitespace handling
    if (isWhitespace(currentChar)) {
        std::string val = "";
        while (currentChar != EOF && isWhitespace(currentChar)) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::DELETE_TOKEN, currentLine);
    }

    // 2. Single-line comment handling (Screener)
    if (currentChar == '/' && peek() == '/') {
        std::string val = "";
        while (currentChar != EOF && currentChar != '\n') {
            val += currentChar;
            advance();
        }
        if (currentChar == '\n') {
            val += currentChar;
            advance(); // Consume the newline
        }
        return Token(val, TokenType::DELETE_TOKEN, currentLine);
    }

    // 3. Identifier scanning (Letters, digits, and underscores allowed after the first letter)
    if (isLetter(currentChar)) {
        std::string val = "";
        int startLine = currentLine;
        while (currentChar != EOF && (isLetter(currentChar) || isDigit(currentChar) || currentChar == '_')) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::IDENTIFIER, startLine);
    }

    // 4. Integer scanning (Sequences of digits)
    if (isDigit(currentChar)) {
        std::string val = "";
        int startLine = currentLine;
        while (currentChar != EOF && isDigit(currentChar)) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::INTEGER, startLine);
    }

    // 5. String literal scanning (Enclosed in single quotes with escape character translation)
    if (currentChar == '\'') {
        std::string val = "";
        int startLine = currentLine;
        advance(); // Consume the opening quote
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
            advance(); // Consume the closing quote
        }
        return Token(val, TokenType::STRING, startLine);
    }

    // 6. Operator scanning (Greedily consumes sequences of operator symbols)
    if (isOperatorSymbol(currentChar)) {
        std::string val = "";
        int startLine = currentLine;
        while (currentChar != EOF && isOperatorSymbol(currentChar)) {
            val += currentChar;
            advance();
        }
        return Token(val, TokenType::OPERATOR, startLine);
    }

    // 7. Punctuation scanning
    if (isPunctuation(currentChar)) {
        std::string val = std::string(1, currentChar);
        int startLine = currentLine;
        advance();
        return Token(val, TokenType::PUNCTUATION, startLine);
    }

    // 8. Unknown character fallback
    std::string val = std::string(1, currentChar);
    int startLine = currentLine;
    advance();
    return Token(val, TokenType::UNKNOWN, startLine);
}

/**
 * @brief Utility for loading all tokens in one pass (useful for unit testing).
 */
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
