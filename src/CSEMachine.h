#ifndef CSEMACHINE_H
#define CSEMACHINE_H

#include "TreeNode.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iostream>

enum class ItemType {
    INTEGER, STRING, TRUTH_VALUE, DUMMY, NIL,
    IDENTIFIER, OPERATOR, 
    LAMBDA, GAMMA, ENV_MARKER, CLOSURE, ETA_CLOSURE,
    TUPLE, PRIMITIVE_FUNC
};

class Environment;
class CSEItem;

class CSEItem {
public:
    ItemType type;
    std::string value;
    int delta_index;
    std::shared_ptr<Environment> env_ptr;
    std::string bound_var;
    std::vector<std::string> bound_vars; // For multiple vars in tuple
    std::vector<std::shared_ptr<CSEItem>> tuple_items;
    std::shared_ptr<Environment> previous_env; // For env_marker to restore the caller's environment
    std::shared_ptr<CSEItem> closure_ptr; // For eta-closure

    CSEItem(ItemType t, std::string v = "") : type(t), value(v), delta_index(-1), env_ptr(nullptr) {}
};

class Environment {
public:
    int id;
    std::shared_ptr<Environment> parent;
    std::map<std::string, std::shared_ptr<CSEItem>> bindings;

    Environment(int env_id, std::shared_ptr<Environment> p) : id(env_id), parent(p) {}
    
    std::shared_ptr<CSEItem> lookup(const std::string& name) {
        if (bindings.find(name) != bindings.end()) return bindings[name];
        if (parent) return parent->lookup(name);
        return nullptr; // not found
    }
};

class CSEMachine {
private:
    std::vector<std::vector<std::shared_ptr<CSEItem>>> deltas;
    std::vector<std::shared_ptr<CSEItem>> control;
    std::vector<std::shared_ptr<CSEItem>> stack;
    std::shared_ptr<Environment> current_env;
    std::shared_ptr<Environment> PE;
    int env_counter;

    void flatten(std::shared_ptr<TreeNode> node, int current_delta_index);
    std::shared_ptr<CSEItem> createItemFromNode(std::shared_ptr<TreeNode> node);
    
    void applyPrimitive(const std::string& func_name);
    void applyBinaryOp(const std::string& op);
    void applyUnaryOp(const std::string& op);
    std::string escapeString(const std::string& s);
    void printItem(std::shared_ptr<CSEItem> item, bool is_conc = false);

public:
    CSEMachine(std::shared_ptr<TreeNode> root);
    void evaluate();
};

#endif // CSEMACHINE_H
