#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "TreeNode.h"
#include <vector>
#include <memory>
#include <string>

class Parser {
private:
    Lexer lexer;
    Token currentToken;
    std::vector<std::shared_ptr<TreeNode>> treeStack;

    void read(const std::string& expectedValue);
    void readToken(TokenType expectedType);
    void buildTree(const std::string& type, const std::string& value, int numChildren);
    bool isKeyword(const std::string& val);

    // Phrase Structure Grammar Methods
    void E();
    void Ew();
    void T();
    void Ta();
    void Tc();
    void B();
    void Bt();
    void Bs();
    void Bp();
    void A();
    void At();
    void Af();
    void Ap();
    void R();
    void Rn();
    void D();
    void Da();
    void Dr();
    void Db();
    void Vb();
    void Vl();

public:
    Parser(const std::string& filename);
    std::shared_ptr<TreeNode> parse();
    void printAST(std::shared_ptr<TreeNode> node, int depth = 0);
};

#endif // PARSER_H
