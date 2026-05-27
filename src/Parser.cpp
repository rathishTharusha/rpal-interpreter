#include "Parser.h"
#include <iostream>
#include <stdexcept>

/**
 * @brief Constructor. Initializes the parser with a lexer and fetches the first lookahead token.
 * @param filename Source file path.
 */
Parser::Parser(const std::string& filename) : lexer(filename), currentToken("", TokenType::UNKNOWN, 0) {
    currentToken = lexer.getNextToken();
}

/**
 * @brief Checks if a string is a reserved keyword in the RPAL grammar.
 */
bool Parser::isKeyword(const std::string& val) {
    static const std::vector<std::string> keywords = {
        "let", "in", "fn", "where", "aug", "or", "not", "gr", "ge", "ls", "le", "eq", "ne", "within", "and", "rec"
    };
    for (const auto& k : keywords) {
        if (val == k) return true;
    }
    return false;
}

/**
 * @brief Validates and consumes the current token if its string value matches the expected value.
 * Fetches the next token from the lexer lookahead.
 */
void Parser::read(const std::string& expectedValue) {
    if (currentToken.value == expectedValue) {
        currentToken = lexer.getNextToken();
    } else {
        throw std::runtime_error("Expected '" + expectedValue + "' but found '" + currentToken.value + "' at line " + std::to_string(currentToken.lineNumber));
    }
}

/**
 * @brief Validates and consumes a terminal token of the expected category.
 * Creates a leaf node representing the terminal and pushes it onto the parse stack.
 */
void Parser::readToken(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        std::string val = currentToken.value;
        std::string nodeType;
        if (expectedType == TokenType::IDENTIFIER) {
            // Ensure keywords or special built-in constants aren't read as user identifiers
            if (isKeyword(val) || val == "true" || val == "false" || val == "nil" || val == "dummy") {
                throw std::runtime_error("Expected IDENTIFIER but found keyword '" + val + "' at line " + std::to_string(currentToken.lineNumber));
            }
            nodeType = "<IDENTIFIER>";
        }
        else if (expectedType == TokenType::INTEGER) nodeType = "<INTEGER>";
        else if (expectedType == TokenType::STRING) nodeType = "<STRING>";
        else nodeType = "UNKNOWN";

        buildTree(nodeType, val, 0); // Terminals have 0 children
        currentToken = lexer.getNextToken();
    } else {
        throw std::runtime_error("Expected token type but found '" + currentToken.value + "' at line " + std::to_string(currentToken.lineNumber));
    }
}

/**
 * @brief Assembles a sub-tree of the AST bottom-up.
 * Pops numChildren nodes from the stack, links them as siblings in lexical order
 * (reversing the stack's right-to-left order), and attaches the chain to a new parent node.
 * Finally, pushes the parent back onto the stack.
 */
void Parser::buildTree(const std::string& type, const std::string& value, int numChildren) {
    auto node = std::make_shared<TreeNode>(type, value);
    std::shared_ptr<TreeNode> child = nullptr;
    while (numChildren > 0) {
        if (treeStack.empty()) throw std::runtime_error("Tree stack underflow");
        auto c = treeStack.back();
        treeStack.pop_back();
        c->setSibling(child); // Link previously popped nodes as siblings on the right
        child = c;            // Current node becomes the leftmost child of the sibling chain
        numChildren--;
    }
    node->setChild(child);
    treeStack.push_back(node);
}

/**
 * @brief Initiates parsing. Validates that the entire file has been consumed.
 * @return The root node of the parsed AST.
 */
std::shared_ptr<TreeNode> Parser::parse() {
    E(); // Parse starting expression
    if (currentToken.type != TokenType::END_OF_FILE) {
        throw std::runtime_error("Expected EOF, but found '" + currentToken.value + "'");
    }
    if (treeStack.size() != 1) {
        throw std::runtime_error("Parse error: multiple roots on tree stack");
    }
    return treeStack.back();
}

/**
 * @brief Recursively prints the AST in pre-order with dot-indented format.
 */
void Parser::printAST(std::shared_ptr<TreeNode> node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) std::cout << ".";
    std::cout << node->type;
    if (node->type == "<IDENTIFIER>" || node->type == "<INTEGER>" || node->type == "<STRING>") {
        std::cout << ":" << node->value;
    }
    std::cout << std::endl;
    printAST(node->child, depth + 1); // Indent child
    printAST(node->sibling, depth);    // Sibling stays at the same depth
}

// ============ Recursive Descent Grammar Rule Methods ============

/**
 * @brief Parses Expressions (E)
 * E -> 'let' D 'in' E        => 'let'
 *   -> 'fn' Vb+ '.' E        => 'lambda'
 *   -> Ew
 */
void Parser::E() {
    if (currentToken.value == "let") {
        read("let");
        D();
        read("in");
        E();
        buildTree("let", "", 2);
    } else if (currentToken.value == "fn") {
        read("fn");
        int n = 0;
        do {
            Vb();
            n++;
        } while (currentToken.value == "(" || currentToken.type == TokenType::IDENTIFIER);
        read(".");
        E();
        buildTree("lambda", "", n + 1);
    } else {
        Ew();
    }
}

/**
 * @brief Parses Where-clauses (Ew)
 * Ew -> T 'where' Dr         => 'where'
 *    -> T
 */
void Parser::Ew() {
    T();
    if (currentToken.value == "where") {
        read("where");
        Dr();
        buildTree("where", "", 2);
    }
}

/**
 * @brief Parses Tuples (T)
 * T -> Ta ( ',' Ta )+        => 'tau'
 *   -> Ta
 */
void Parser::T() {
    Ta();
    int n = 0;
    while (currentToken.value == ",") {
        read(",");
        Ta();
        n++;
    }
    if (n > 0) {
        buildTree("tau", "", n + 1);
    }
}

/**
 * @brief Parses Tuple Joins (Ta)
 * Ta -> Tc ( 'aug' Tc )*     => 'aug'
 */
void Parser::Ta() {
    Tc();
    while (currentToken.value == "aug") {
        read("aug");
        Tc();
        buildTree("aug", "", 2);
    }
}

/**
 * @brief Parses Conditional Expressions (Tc)
 * Tc -> B '->' Tc '|' Tc     => '->'
 *    -> B
 */
void Parser::Tc() {
    B();
    if (currentToken.value == "->") {
        read("->");
        Tc();
        read("|");
        Tc();
        buildTree("->", "", 3);
    }
}

/**
 * @brief Parses Boolean OR Expressions (B)
 * B -> Bt ( 'or' Bt )*       => 'or'
 */
void Parser::B() {
    Bt();
    while (currentToken.value == "or") {
        read("or");
        Bt();
        buildTree("or", "", 2);
    }
}

/**
 * @brief Parses Boolean AND Expressions (Bt)
 * Bt -> Bs ( '&' Bs )*       => '&'
 */
void Parser::Bt() {
    Bs();
    while (currentToken.value == "&") {
        read("&");
        Bs();
        buildTree("&", "", 2);
    }
}

/**
 * @brief Parses Boolean Unary Negations (Bs)
 * Bs -> 'not' Bp             => 'not'
 *    -> Bp
 */
void Parser::Bs() {
    if (currentToken.value == "not") {
        read("not");
        Bp();
        buildTree("not", "", 1);
    } else {
        Bp();
    }
}

/**
 * @brief Parses Comparisons (Bp)
 * Bp -> A ( 'gr' | '>' ) A   => 'gr'
 *    -> A ( 'ge' | '>=' ) A  => 'ge'
 *    -> A ( 'ls' | '<' ) A   => 'ls'
 *    -> A ( 'le' | '<=' ) A  => 'le'
 *    -> A 'eq' A             => 'eq'
 *    -> A 'ne' A             => 'ne'
 *    -> A
 */
void Parser::Bp() {
    A();
    if (currentToken.value == "gr" || currentToken.value == ">") {
        std::string op = currentToken.value == "gr" ? "gr" : ">";
        read(currentToken.value);
        A();
        buildTree(op, "", 2);
    } else if (currentToken.value == "ge" || currentToken.value == ">=") {
        std::string op = currentToken.value == "ge" ? "ge" : ">=";
        read(currentToken.value);
        A();
        buildTree(op, "", 2);
    } else if (currentToken.value == "ls" || currentToken.value == "<") {
        std::string op = currentToken.value == "ls" ? "ls" : "<";
        read(currentToken.value);
        A();
        buildTree(op, "", 2);
    } else if (currentToken.value == "le" || currentToken.value == "<=") {
        std::string op = currentToken.value == "le" ? "le" : "<=";
        read(currentToken.value);
        A();
        buildTree(op, "", 2);
    } else if (currentToken.value == "eq") {
        read("eq");
        A();
        buildTree("eq", "", 2);
    } else if (currentToken.value == "ne") {
        read("ne");
        A();
        buildTree("ne", "", 2);
    }
}

/**
 * @brief Parses Arithmetic Additions and Subtractions (A)
 * A -> '+' At
 *   -> '-' At                => 'neg'
 *   -> At
 *   followed by ( '+' At | '-' At )* => '+' | '-'
 */
void Parser::A() {
    if (currentToken.value == "+") {
        read("+");
        At();
    } else if (currentToken.value == "-") {
        read("-");
        At();
        buildTree("neg", "", 1);
    } else {
        At();
    }
    
    while (currentToken.value == "+" || currentToken.value == "-") {
        std::string op = currentToken.value;
        read(op);
        At();
        buildTree(op, "", 2);
    }
}

/**
 * @brief Parses Arithmetic Multiplications and Divisions (At)
 * At -> Af ( ( '*' | '/' ) Af )*  => '*' | '/'
 */
void Parser::At() {
    Af();
    while (currentToken.value == "*" || currentToken.value == "/") {
        std::string op = currentToken.value;
        read(op);
        Af();
        buildTree(op, "", 2);
    }
}

/**
 * @brief Parses Exponentiations (Af)
 * Af -> Ap '**' Af           => '**' (right associative)
 *    -> Ap
 */
void Parser::Af() {
    Ap();
    if (currentToken.value == "**") {
        read("**");
        Af();
        buildTree("**", "", 2);
    }
}

/**
 * @brief Parses Infix User-defined Function Application (Ap)
 * Ap -> R ( '@' IDENTIFIER R )* => '@'
 */
void Parser::Ap() {
    R();
    while (currentToken.value == "@") {
        read("@");
        readToken(TokenType::IDENTIFIER);
        R();
        buildTree("@", "", 3);
    }
}

/**
 * @brief Parses Function Application (R)
 * R -> R Rn                  => 'gamma' (left associative)
 *   -> Rn
 */
void Parser::R() {
    Rn();
    while ((currentToken.type == TokenType::IDENTIFIER && !isKeyword(currentToken.value) && 
            currentToken.value != "true" && currentToken.value != "false" && 
            currentToken.value != "nil" && currentToken.value != "dummy") || 
           currentToken.type == TokenType::INTEGER || 
           currentToken.type == TokenType::STRING || currentToken.value == "true" || 
           currentToken.value == "false" || currentToken.value == "nil" || 
           currentToken.value == "dummy" || currentToken.value == "(") {
        Rn();
        buildTree("gamma", "", 2);
    }
}

/**
 * @brief Parses Terminals and Basic Blocks (Rn)
 * Rn -> 'true'                => 'true'
 *    -> 'false'               => 'false'
 *    -> 'nil'                 => 'nil'
 *    -> 'dummy'               => 'dummy'
 *    -> '(' E ')'
 *    -> IDENTIFIER            => '<IDENTIFIER>'
 *    -> INTEGER               => '<INTEGER>'
 *    -> STRING                => '<STRING>'
 */
void Parser::Rn() {
    if (currentToken.value == "true") {
        read("true");
        buildTree("true", "", 0);
    } else if (currentToken.value == "false") {
        read("false");
        buildTree("false", "", 0);
    } else if (currentToken.value == "nil") {
        read("nil");
        buildTree("nil", "", 0);
    } else if (currentToken.value == "dummy") {
        read("dummy");
        buildTree("dummy", "", 0);
    } else if (currentToken.value == "(") {
        read("(");
        E();
        read(")");
    } else if (currentToken.type == TokenType::IDENTIFIER) {
        readToken(TokenType::IDENTIFIER);
    } else if (currentToken.type == TokenType::INTEGER) {
        readToken(TokenType::INTEGER);
    } else if (currentToken.type == TokenType::STRING) {
        readToken(TokenType::STRING);
    } else {
        throw std::runtime_error("Unexpected token in Rn(): " + currentToken.value + " at line " + std::to_string(currentToken.lineNumber));
    }
}

/**
 * @brief Parses Declarations (D)
 * D -> Da 'within' D         => 'within'
 *   -> Da
 */
void Parser::D() {
    Da();
    if (currentToken.value == "within") {
        read("within");
        D();
        buildTree("within", "", 2);
    }
}

/**
 * @brief Parses Simultaneous Declarations (Da)
 * Da -> Dr ( 'and' Dr )+     => 'and'
 *    -> Dr
 */
void Parser::Da() {
    Dr();
    int n = 0;
    while (currentToken.value == "and") {
        read("and");
        Dr();
        n++;
    }
    if (n > 0) {
        buildTree("and", "", n + 1);
    }
}

/**
 * @brief Parses Recursive Declarations (Dr)
 * Dr -> 'rec' Db             => 'rec'
 *    -> Db
 */
void Parser::Dr() {
    if (currentToken.value == "rec") {
        read("rec");
        Db();
        buildTree("rec", "", 1);
    } else {
        Db();
    }
}

/**
 * @brief Parses Definition Bindings (Db)
 * Db -> '(' D ')'
 *    -> IDENTIFIER Vb+ '=' E  => 'fcn_form'
 *    -> IDENTIFIER ',' Vl '=' E => '='
 *    -> IDENTIFIER '=' E      => '='
 */
void Parser::Db() {
    if (currentToken.value == "(") {
        read("(");
        D();
        read(")");
    } else if (currentToken.type == TokenType::IDENTIFIER) {
        readToken(TokenType::IDENTIFIER);
        if (currentToken.value == "," || currentToken.value == "=") {
            Vl(); // Parse comma-separated list if it exists
            read("=");
            E();
            buildTree("=", "", 2);
        } else {
            // Function form declaration (curried headers)
            int n = 0;
            while (currentToken.type == TokenType::IDENTIFIER || currentToken.value == "(") {
                Vb();
                n++;
            }
            read("=");
            E();
            buildTree("fcn_form", "", n + 2); // Parent form + parameters + E
        }
    } else {
        throw std::runtime_error("Unexpected token in Db(): " + currentToken.value);
    }
}

/**
 * @brief Parses Variable Bindings (Vb)
 * Vb -> IDENTIFIER            => '<IDENTIFIER>'
 *    -> '(' ')'               => '()'
 *    -> '(' IDENTIFIER Vl ')'
 */
void Parser::Vb() {
    if (currentToken.type == TokenType::IDENTIFIER) {
        readToken(TokenType::IDENTIFIER);
    } else if (currentToken.value == "(") {
        read("(");
        if (currentToken.value == ")") {
            read(")");
            buildTree("()", "", 0);
        } else {
            readToken(TokenType::IDENTIFIER);
            Vl();
            read(")");
        }
    } else {
        throw std::runtime_error("Unexpected token in Vb(): " + currentToken.value);
    }
}

/**
 * @brief Parses Variable Lists (Vl)
 * Vl -> ',' IDENTIFIER ( ',' IDENTIFIER )*  => ','
 */
void Parser::Vl() {
    int n = 0;
    while (currentToken.value == ",") {
        read(",");
        readToken(TokenType::IDENTIFIER);
        n++;
    }
    if (n > 0) {
        buildTree(",", "", n + 1);
    }
}
