#ifndef CSEMACHINE_H
#define CSEMACHINE_H

#include "TreeNode.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iostream>

/**
 * @brief Represents the types of items that can be stored on the stack or control structures.
 */
enum class ItemType {
    INTEGER,          // Numeric constant (e.g. 5)
    STRING,           // String constant (e.g. 'hello')
    TRUTH_VALUE,      // Boolean constants (true or false)
    DUMMY,            // Dummy value for parameterless functions/closures
    NIL,              // Empty list/tuple marker
    IDENTIFIER,       // Variable names waiting to be resolved in an environment
    OPERATOR,         // Built-in operators (+, -, eq, beta, etc.)
    LAMBDA,           // Lambda abstraction marker pointing to a control structure delta index
    GAMMA,            // Function application operator
    ENV_MARKER,       // Environment marker (e.g. e_i) to signify environment boundaries
    CLOSURE,          // Evaluated function representation containing parameter names, delta index, and parent scope
    ETA_CLOSURE,      // Recursion closure wrapping for Y-combinator self-application
    TUPLE,            // Multi-value compound item constructed by tau
    PRIMITIVE_FUNC    // Built-in primitive function (e.g. Print, Conc, Stem, Stern, aug)
};

class Environment;
class CSEItem;

/**
 * @brief Represents an entry on the Control stack or the Value stack.
 */
class CSEItem {
public:
    ItemType type;                               // Item category
    std::string value;                           // string representation of the value
    int delta_index;                             // Delta index containing code block for LAMBDA/CLOSURE
    std::shared_ptr<Environment> env_ptr;        // Environment binding pointer for CLOSURE/ENV_MARKER
    std::string bound_var;                       // Bound parameter identifier name for lambda
    std::vector<std::string> bound_vars;         // Parameter list for tuple-destructuring lambdas
    std::vector<std::shared_ptr<CSEItem>> tuple_items; // Child items for evaluated TUPLEs
    std::shared_ptr<Environment> previous_env;   // Environment pointer to restore on ENV_MARKER pop
    std::shared_ptr<CSEItem> closure_ptr;        // Closure reference wrapped by an ETA_CLOSURE

    CSEItem(ItemType t, std::string v = "") : type(t), value(v), delta_index(-1), env_ptr(nullptr) {}
};

/**
 * @brief Represents a scope/environment frame holding variable-to-value bindings.
 * Supports lexical scope nesting by linking to a parent environment.
 */
class Environment {
public:
    int id;                                                   // Environment counter ID (0 for primitive E0)
    std::shared_ptr<Environment> parent;                      // Parent scope pointer (lexical ancestor)
    std::map<std::string, std::shared_ptr<CSEItem>> bindings; // Scope-local identifier bindings

    Environment(int env_id, std::shared_ptr<Environment> p) : id(env_id), parent(p) {}
    
    // Recursively searches for an identifier in the current and ancestor environment frames
    std::shared_ptr<CSEItem> lookup(const std::string& name) {
        if (bindings.find(name) != bindings.end()) return bindings[name];
        if (parent) return parent->lookup(name);
        return nullptr; // Identifier not declared
    }
};

/**
 * @brief Control Stack Evaluator (CSE) Machine engine.
 * 
 * Flattens the Standardized AST into instruction blocks (deltas) and executes
 * the machine's evaluation cycle by popping instructions off the Control stack,
 * updating the Value stack, and managing Environment frames.
 */
class CSEMachine {
private:
    std::vector<std::vector<std::shared_ptr<CSEItem>>> deltas; // Code structures (instructions) index
    std::vector<std::shared_ptr<CSEItem>> control;             // Control Stack (C)
    std::vector<std::shared_ptr<CSEItem>> stack;               // Value Stack (S)
    std::shared_ptr<Environment> current_env;                  // Current active environment (E_i)
    std::shared_ptr<Environment> PE;                           // Primitive environment frame (E0)
    int env_counter;                                           // Unique identifier generator for environments

    // Flattens the standardized tree recursively into sequential instructions grouped into deltas
    void flatten(std::shared_ptr<TreeNode> node, int current_delta_index);
    
    // Helper to map an AST TreeNode to a CSEItem representation
    std::shared_ptr<CSEItem> createItemFromNode(std::shared_ptr<TreeNode> node);
    
    // Primitive built-in function handlers
    void applyPrimitive(const std::string& func_name);
    void applyBinaryOp(const std::string& op);
    void applyUnaryOp(const std::string& op);
    
    // Formats single-quoted string literal escape sequences back for stdout
    std::string escapeString(const std::string& s);
    
    // Formats and prints an item to standard output
    void printItem(std::shared_ptr<CSEItem> item, bool is_conc = false);

public:
    // Flattens the standardized tree and initializes E0, Control, and Stack
    CSEMachine(std::shared_ptr<TreeNode> root);
    
    // Evaluates instructions until Control is empty
    void evaluate();
};

#endif // CSEMACHINE_H
