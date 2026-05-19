#include "Lexer.h"
#include <iostream>

std::string getTokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::STRING: return "STRING";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::PUNCTUATION: return "PUNCTUATION";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::DELETE_TOKEN: return "DELETE";
        case TokenType::UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./rpal20 <filename>\n";
        return 1;
    }

    std::string filename = argv[1];
    
    try {
        Lexer lexer(filename);
        std::vector<Token> tokens = lexer.getAllTokens();
        
        for (const auto& token : tokens) {
            std::cout << "<" << getTokenTypeName(token.type) << "> : " << token.value << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
