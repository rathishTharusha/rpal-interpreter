#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "TreeNode.h"
#include <vector>
#include <memory>
#include <string>

/**
 * @brief LL(1) Predictive Recursive Descent Parser for RPAL.
 * 
 * Analyzes a stream of tokens from the Lexer, verifies grammatical structure,
 * and constructs an Abstract Syntax Tree (AST) bottom-up using a parsing stack.
 */
class Parser {
private:
    Lexer lexer;                                       // The lexical scanner
    Token currentToken;                                // Single-token lookahead buffer
    std::vector<std::shared_ptr<TreeNode>> treeStack;  // Accumulator stack for sub-tree assembly

    // Consumes currentToken if it matches the expected lexeme value, otherwise throws syntax error
    void read(const std::string& expectedValue);
    
    // Consumes currentToken of the expected type, and builds a corresponding terminal leaf node
    void readToken(TokenType expectedType);
    
    // Pops n elements from the tree stack, chains them as siblings, and attaches them under a new parent node
    void buildTree(const std::string& type, const std::string& value, int numChildren);
    
    // Utility to determine if an identifier is a reserved keyword in RPAL
    bool isKeyword(const std::string& val);

    // Mutual recursion methods representing the Phrase Structure Grammar non-terminals:
    void E();   // Expressions (let, fn, or nested)
    void Ew();  // Where-clauses
    void T();   // Tuples (comma-separated lists)
    void Ta();  // List joins (aug)
    void Tc();  // Conditional branching (->)
    void B();   // Boolean OR
    void Bt();  // Boolean AND
    void Bs();  // Unary NOT
    void Bp();  // Comparison operators (gr, ge, ls, le, eq, ne)
    void A();   // Addition and subtraction (+, -)
    void At();  // Multiplication and division (*, /)
    void Af();  // Exponents (**)
    void Ap();  // Function application (infix @)
    void R();   // Primary operators and prefix functions
    void Rn();  // Terminals and basic blocks (integers, strings, identifiers, parentheses)
    void D();   // Declarations (within)
    void Da();  // Simultaneous declarations (and)
    void Dr();  // Recursive declarations (rec)
    void Db();  // Simple bindings and function headers
    void Vb();  // Variables (identifiers or nested parameter tuples)
    void Vl();  // Variable lists (comma-separated parameters)

public:
    // Initializes lookahead by loading the first token from the scanner
    Parser(const std::string& filename);
    
    // Parses the program, returning the root node of the generated AST
    std::shared_ptr<TreeNode> parse();
    
    // Utility to print the tree hierarchy in indented dot-prefix form (e.g. .let, ..lambda)
    void printAST(std::shared_ptr<TreeNode> node, int depth = 0);
};

#endif // PARSER_H
