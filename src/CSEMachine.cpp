#include "CSEMachine.h"
#include <stdexcept>
#include <cmath>

/**
 * @brief Constructor. Establishes the Primitive Environment (E0), flattens the
 * standardized tree into instructions, and primes the Control stack and Value stack.
 * @param root Root node of the standardized tree.
 */
CSEMachine::CSEMachine(std::shared_ptr<TreeNode> root) : env_counter(0) {
    // E0 is the root environment frame (no parent frame)
    PE = std::make_shared<Environment>(env_counter++, nullptr);
    current_env = PE;

    // Allocate first instruction block (delta 0)
    deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
    flatten(root, 0); // Flatten AST tree starting at delta 0

    // e0 environment boundary marker
    auto env_item = std::make_shared<CSEItem>(ItemType::ENV_MARKER, "0");
    env_item->env_ptr = PE;

    // Control Stack: Push environment marker first, then delta 0 instructions in reverse
    control.push_back(env_item);
    for (auto it = deltas[0].rbegin(); it != deltas[0].rend(); ++it) {
        control.push_back(*it);
    }
    
    // Value Stack: Initialize with the environment marker e0
    stack.push_back(env_item);
}

/**
 * @brief Post-order traversal to flatten the standardized tree into instruction streams.
 * Maps lambda and conditional nodes into separate instruction blocks (deltas)
 * to facilitate branching and closure definitions.
 */
void CSEMachine::flatten(std::shared_ptr<TreeNode> node, int current_delta_index) {
    if (!node) return;
    
    if (node->type == "lambda") {
        // Lambda case: Create a new delta instruction block for the lambda body
        int next_delta = deltas.size();
        deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
        flatten(node->child->sibling, next_delta); // Flatten lambda body E
        
        auto item = std::make_shared<CSEItem>(ItemType::LAMBDA);
        item->delta_index = next_delta;
        
        // Extract bound parameter variable name(s)
        auto var_node = node->child;
        if (var_node->type == ",") {
            // Destructured multi-parameter lambda
            auto c = var_node->child;
            while(c) {
                item->bound_vars.push_back(c->value);
                c = c->sibling;
            }
        } else {
            // Standard single-parameter lambda
            item->bound_var = var_node->value;
        }
        deltas[current_delta_index].push_back(item);
    } else if (node->type == "->") {
        // Conditional case: Flatten condition B, and create separate deltas for Then (T) and Else (E)
        auto B = node->child;
        auto T = B->sibling;
        auto E = T->sibling;
        
        flatten(B, current_delta_index); // Flatten condition block onto active control stream
        
        int delta_then = deltas.size();
        deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
        flatten(T, delta_then);          // Flatten Then-branch E
        
        int delta_else = deltas.size();
        deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
        flatten(E, delta_else);          // Flatten Else-branch E
        
        // Push a custom beta operator recording the indices of both branches
        auto beta = std::make_shared<CSEItem>(ItemType::OPERATOR, "beta");
        beta->bound_vars.push_back(std::to_string(delta_then));
        beta->bound_vars.push_back(std::to_string(delta_else));
        deltas[current_delta_index].push_back(beta);
    } else {
        // Standard n-ary node: Flatten children from left to right, then push the operator node
        auto c = node->child;
        while (c) {
            flatten(c, current_delta_index);
            c = c->sibling;
        }
        deltas[current_delta_index].push_back(createItemFromNode(node));
    }
}

/**
 * @brief Translates an AST TreeNode to a corresponding virtual machine CSEItem.
 */
std::shared_ptr<CSEItem> CSEMachine::createItemFromNode(std::shared_ptr<TreeNode> node) {
    if (node->type == "gamma") return std::make_shared<CSEItem>(ItemType::GAMMA);
    if (node->type == "<INTEGER>") return std::make_shared<CSEItem>(ItemType::INTEGER, node->value);
    if (node->type == "<STRING>") return std::make_shared<CSEItem>(ItemType::STRING, node->value);
    if (node->type == "true" || node->type == "false") return std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, node->type);
    if (node->type == "nil") return std::make_shared<CSEItem>(ItemType::NIL);
    if (node->type == "dummy" || node->type == "()") return std::make_shared<CSEItem>(ItemType::DUMMY);
    if (node->type == "Ystar") return std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Ystar");
    if (node->type == "tau") {
        auto item = std::make_shared<CSEItem>(ItemType::TUPLE, "tau");
        int count = 0;
        auto c = node->child;
        while (c) { count++; c = c->sibling; }
        item->bound_vars.push_back(std::to_string(count)); // Store tuple element count
        return item;
    }
    if (node->type == "<IDENTIFIER>") {
        std::string v = node->value;
        // Intercept built-in functions
        if (v == "Print" || v == "Stern" || v == "Stem" || v == "Conc" || v == "Order" || v == "Null" || 
            v == "Isinteger" || v == "Isstring" || v == "Istuple" || v == "Istruthvalue" || v == "Isfunction" || v == "ItoS" || v == "Cond" || v == "aug" || v == "Ystar") {
            return std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, v);
        }
        return std::make_shared<CSEItem>(ItemType::IDENTIFIER, v);
    }
    return std::make_shared<CSEItem>(ItemType::OPERATOR, node->type);
}

/**
 * @brief Primary evaluation loop. Executes until the control stack is empty.
 * Matches instructions and implements the 13 execution rules of the CSE machine.
 */
void CSEMachine::evaluate() {
    while (!control.empty()) {
        auto item = control.back();
        control.pop_back();

        // 1. Constants and closure representations are pushed directly onto the value stack
        if (item->type == ItemType::INTEGER || item->type == ItemType::STRING || 
            item->type == ItemType::TRUTH_VALUE || item->type == ItemType::DUMMY || 
            item->type == ItemType::NIL || item->type == ItemType::CLOSURE || 
            item->type == ItemType::PRIMITIVE_FUNC || 
            (item->type == ItemType::TUPLE && item->value != "tau") || 
            item->type == ItemType::ETA_CLOSURE) {
            stack.push_back(item);
        }
        // 2. Identifier Name Lookup (Rule 1)
        else if (item->type == ItemType::IDENTIFIER) {
            auto val = current_env->lookup(item->value);
            if (!val) {
                if (item->value == "Print") {
                    stack.push_back(std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Print"));
                } else {
                    throw std::runtime_error("Undeclared identifier: " + item->value);
                }
            } else {
                stack.push_back(val);
            }
        }
        // 3. Lambda Definition (Rule 3)
        else if (item->type == ItemType::LAMBDA) {
            auto closure = std::make_shared<CSEItem>(ItemType::CLOSURE);
            closure->env_ptr = current_env; // Bind closure to definition scope
            closure->delta_index = item->delta_index;
            closure->bound_var = item->bound_var;
            closure->bound_vars = item->bound_vars;
            stack.push_back(closure);
        }
        // 4. Function Application (Rule 4 / Gamma)
        else if (item->type == ItemType::GAMMA) {
            if (stack.size() < 2) {
                throw std::runtime_error("Stack underflow in GAMMA operator");
            }
            auto rand = stack.back(); stack.pop_back();   // Operand
            auto rator = stack.back(); stack.pop_back();  // Operator

            if (rator->type == ItemType::CLOSURE) {
                // Apply closure: Create a new scope, bind variables, and restore caller's scope later
                auto new_env = std::make_shared<Environment>(env_counter++, rator->env_ptr);
                
                if (rator->bound_vars.size() > 0 && rand->type == ItemType::TUPLE) {
                    // Handle multi-parameter tuple destructuring (e.g. lambda (x, y))
                    for (size_t i = 0; i < rator->bound_vars.size(); ++i) {
                        new_env->bindings[rator->bound_vars[i]] = rand->tuple_items[i];
                    }
                } else {
                    // Standard single parameter binding
                    new_env->bindings[rator->bound_var] = rand;
                }
                
                auto env_marker = std::make_shared<CSEItem>(ItemType::ENV_MARKER, std::to_string(new_env->id));
                env_marker->env_ptr = new_env;
                env_marker->previous_env = current_env; // Save caller environment
                current_env = new_env;
                
                control.push_back(env_marker);
                stack.push_back(env_marker);
                
                // Push lambda body instructions onto Control stack in reverse order
                auto& d = deltas[rator->delta_index];
                for (auto it = d.rbegin(); it != d.rend(); ++it) {
                    control.push_back(*it);
                }
            } else if (rator->type == ItemType::PRIMITIVE_FUNC) {
                // Apply primitive function
                if (rator->value == "Ystar") {
                    // Ystar initialization (Rule 11) - wrap in eta-closure
                    auto eta = std::make_shared<CSEItem>(ItemType::ETA_CLOSURE);
                    eta->closure_ptr = rand;
                    stack.push_back(eta);
                } else if (rator->value == "Cond") {
                    // Conditional wrapper: collect condition B
                    auto partial1 = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Cond1");
                    partial1->tuple_items.push_back(rand);
                    stack.push_back(partial1);
                } else if (rator->value == "Cond1") {
                    // Conditional wrapper: collect Then branch T
                    auto partial2 = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Cond2");
                    partial2->tuple_items.push_back(rator->tuple_items[0]); // B
                    partial2->tuple_items.push_back(rand);                  // T
                    stack.push_back(partial2);
                } else if (rator->value == "Cond2") {
                    // Conditional wrapper: evaluate conditional branch (lazy execution)
                    auto B = rator->tuple_items[0];
                    auto T = rator->tuple_items[1];
                    auto E = rand;
                    auto to_execute = (B->value == "true") ? T : E;
                    
                    // Evaluate lazy closure by applying to dummy
                    control.push_back(std::make_shared<CSEItem>(ItemType::GAMMA));
                    stack.push_back(to_execute);
                    stack.push_back(std::make_shared<CSEItem>(ItemType::DUMMY));
                } else if (rator->value == "Conc1") {
                    // Complete string concatenation
                    auto res = std::make_shared<CSEItem>(ItemType::STRING, rator->tuple_items[0]->value + rand->value);
                    stack.push_back(res);
                } else if (rator->value == "aug1") {
                    // Complete aug appending to tuple or nil list
                    auto tup = rator->tuple_items[0];
                    if (tup->type == ItemType::NIL) {
                        auto new_tup = std::make_shared<CSEItem>(ItemType::TUPLE);
                        new_tup->tuple_items.push_back(rand);
                        stack.push_back(new_tup);
                    } else if (tup->type == ItemType::TUPLE) {
                        auto new_tup = std::make_shared<CSEItem>(ItemType::TUPLE);
                        new_tup->tuple_items = tup->tuple_items;
                        new_tup->tuple_items.push_back(rand);
                        stack.push_back(new_tup);
                    } else {
                        throw std::runtime_error("aug on non-tuple type");
                    }
                } else {
                    // Standard single-argument primitives
                    stack.push_back(rand); // Put rand back so applyPrimitive can pop it
                    applyPrimitive(rator->value);
                }
            } else if (rator->type == ItemType::ETA_CLOSURE) {
                // Eta Application recursive step (Rule 12)
                control.push_back(std::make_shared<CSEItem>(ItemType::GAMMA));
                control.push_back(rand);
                control.push_back(std::make_shared<CSEItem>(ItemType::GAMMA));
                stack.push_back(rator->closure_ptr);
                stack.push_back(rator);
            } else if (rator->type == ItemType::TUPLE) {
                // Tuple selection (Rule 10)
                if (rand->type == ItemType::INTEGER) {
                    int idx = std::stoi(rand->value);
                    stack.push_back(rator->tuple_items[idx - 1]);
                }
            }
        }
        // 5. Scope Restore (Rule 5 / e_i marker)
        else if (item->type == ItemType::ENV_MARKER) {
            current_env = item->previous_env; // Restore parent env
            if (stack.size() < 2) throw std::runtime_error("Stack underflow in ENV_MARKER");
            auto result = stack.back(); stack.pop_back(); // Save result
            stack.pop_back();                             // Remove matching ENV_MARKER from stack
            stack.push_back(result);
        }
        // 6. Tuple Construction (Rule 9 / Tau)
        else if (item->type == ItemType::TUPLE && item->value == "tau") {
            int n = std::stoi(item->bound_vars[0]);
            if (stack.size() < (size_t)n) throw std::runtime_error("Stack underflow in tau");
            auto tuple = std::make_shared<CSEItem>(ItemType::TUPLE);
            // Collect n items in reverse stack order
            for (int i = 0; i < n; ++i) {
                tuple->tuple_items.insert(tuple->tuple_items.begin(), stack.back());
                stack.pop_back();
            }
            stack.push_back(tuple);
        }
        // 7. Operators (Rule 6 and Rule 7)
        else if (item->type == ItemType::OPERATOR) {
            if (item->value == "beta") {
                // Conditional Branch selection (Rule 8)
                auto b = stack.back(); stack.pop_back();
                int idx = b->value == "true" ? std::stoi(item->bound_vars[0]) : std::stoi(item->bound_vars[1]);
                auto& d = deltas[idx];
                for (auto it = d.rbegin(); it != d.rend(); ++it) {
                    control.push_back(*it);
                }
            } else if (item->value == "neg" || item->value == "not") {
                applyUnaryOp(item->value);
            } else {
                applyBinaryOp(item->value);
            }
        }
    }
}

/**
 * @brief Executes built-in, single-argument primitive functions.
 */
void CSEMachine::applyPrimitive(const std::string& func_name) {
    if (stack.size() < 1) throw std::runtime_error("Stack underflow in applyPrimitive: " + func_name);
    auto rand = stack.back(); stack.pop_back();
    
    if (func_name == "Print") {
        printItem(rand);
        std::cout << std::endl;
        stack.push_back(std::make_shared<CSEItem>(ItemType::DUMMY));
    } else if (func_name == "Stem") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::STRING, rand->value.substr(0, 1)));
    } else if (func_name == "Stern") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::STRING, rand->value.substr(1)));
    } else if (func_name == "Conc") {
        if (rand->type == ItemType::TUPLE) {
            // Conc applied to tuple of strings (e.g. Conc(S1, S2, ..))
            std::string res = "";
            for (auto& item : rand->tuple_items) {
                res += item->value;
            }
            stack.push_back(std::make_shared<CSEItem>(ItemType::STRING, res));
        } else {
            // Curried string concatenation wait (e.g. Conc S1 S2)
            auto partial = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Conc1");
            partial->tuple_items.push_back(rand); 
            stack.push_back(partial);
        }
    } else if (func_name == "aug") {
        // Curried aug append helper wait
        auto partial = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "aug1");
        partial->tuple_items.push_back(rand); 
        stack.push_back(partial);
    } else if (func_name == "Order") {
        if (rand->type == ItemType::TUPLE) {
            stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(rand->tuple_items.size())));
        } else {
            stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, "0"));
        }
    } else if (func_name == "Isinteger") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand->type == ItemType::INTEGER ? "true" : "false"));
    } else if (func_name == "Isstring") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand->type == ItemType::STRING ? "true" : "false"));
    } else if (func_name == "Istuple") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, (rand->type == ItemType::TUPLE || rand->type == ItemType::NIL) ? "true" : "false"));
    } else if (func_name == "Istruthvalue") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand->type == ItemType::TRUTH_VALUE ? "true" : "false"));
    } else if (func_name == "Isfunction") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand->type == ItemType::CLOSURE ? "true" : "false"));
    } else if (func_name == "Null") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, (rand->type == ItemType::NIL || (rand->type == ItemType::TUPLE && rand->tuple_items.size() == 0)) ? "true" : "false"));
    } else if (func_name == "ItoS") {
        stack.push_back(std::make_shared<CSEItem>(ItemType::STRING, rand->value));
    } else if (func_name == "Cond") {
        auto partial = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Cond1");
        partial->tuple_items.push_back(rand);
        stack.push_back(partial);
    }
}

/**
 * @brief Evaluates binary arithmetic, logical, and structural operations.
 */
void CSEMachine::applyBinaryOp(const std::string& op) {
    if (stack.size() < 2) throw std::runtime_error("Stack underflow in applyBinaryOp: " + op);
    auto rand2 = stack.back(); stack.pop_back();
    auto rand1 = stack.back(); stack.pop_back();

    if (op == "+") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(std::stoi(rand1->value) + std::stoi(rand2->value))));
    else if (op == "-") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(std::stoi(rand1->value) - std::stoi(rand2->value))));
    else if (op == "*") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(std::stoi(rand1->value) * std::stoi(rand2->value))));
    else if (op == "/") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(std::stoi(rand1->value) / std::stoi(rand2->value))));
    else if (op == "**") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string((int)std::pow(std::stoi(rand1->value), std::stoi(rand2->value)))));
    else if (op == "eq") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand1->value == rand2->value ? "true" : "false"));
    else if (op == "ne") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand1->value != rand2->value ? "true" : "false"));
    else if (op == ">" || op == "gr") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, std::stoi(rand1->value) > std::stoi(rand2->value) ? "true" : "false"));
    else if (op == ">=" || op == "ge") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, std::stoi(rand1->value) >= std::stoi(rand2->value) ? "true" : "false"));
    else if (op == "<" || op == "ls") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, std::stoi(rand1->value) < std::stoi(rand2->value) ? "true" : "false"));
    else if (op == "<=" || op == "le") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, std::stoi(rand1->value) <= std::stoi(rand2->value) ? "true" : "false"));
    else if (op == "or") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, (rand1->value == "true" || rand2->value == "true") ? "true" : "false"));
    else if (op == "&") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, (rand1->value == "true" && rand2->value == "true") ? "true" : "false"));
    else if (op == "aug") {
        // Appends rand2 to the end of rand1 tuple
        auto res = std::make_shared<CSEItem>(ItemType::TUPLE);
        if (rand1->type == ItemType::TUPLE) {
            res->tuple_items = rand1->tuple_items;
        } else if (rand1->type != ItemType::NIL) {
            res->tuple_items.push_back(rand1);
        }
        res->tuple_items.push_back(rand2);
        stack.push_back(res);
    } else {
        throw std::runtime_error("Unhandled binary operator: '" + op + "'");
    }
}

/**
 * @brief Evaluates unary operators.
 */
void CSEMachine::applyUnaryOp(const std::string& op) {
    if (stack.size() < 1) throw std::runtime_error("Stack underflow in applyUnaryOp: " + op);
    auto rand = stack.back(); stack.pop_back();
    if (op == "not") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand->value == "true" ? "false" : "true"));
    else if (op == "neg") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(-std::stoi(rand->value))));
}

/**
 * @brief Formats and prints stack items to stdout in standard form.
 */
void CSEMachine::printItem(std::shared_ptr<CSEItem> item, bool is_conc) {
    if (item->type == ItemType::INTEGER) std::cout << item->value;
    else if (item->type == ItemType::STRING) std::cout << escapeString(item->value);
    else if (item->type == ItemType::TRUTH_VALUE) std::cout << item->value;
    else if (item->type == ItemType::DUMMY) std::cout << "dummy";
    else if (item->type == ItemType::NIL) std::cout << "nil";
    else if (item->type == ItemType::CLOSURE) {
        // Displays static delta-index of the lambda body, matching reference interpreter
        std::cout << "[lambda closure: " << item->bound_var << ": " << item->delta_index << "]";
    }
    else if (item->type == ItemType::ETA_CLOSURE) std::cout << "[eta closure]";
    else if (item->type == ItemType::GAMMA) std::cout << "GAMMA ";
    else if (item->type == ItemType::ENV_MARKER) std::cout << "ENV(" << item->value << ") ";
    else if (item->type == ItemType::IDENTIFIER) std::cout << "ID(" << item->value << ") ";
    else if (item->type == ItemType::OPERATOR) std::cout << "OP(" << item->value << ") ";
    else if (item->type == ItemType::LAMBDA) std::cout << "LAMBDA(" << item->delta_index << ") ";
    else if (item->type == ItemType::PRIMITIVE_FUNC) std::cout << "PRIM(" << item->value << ") ";
    else if (item->type == ItemType::TUPLE) {
        std::cout << "(";
        for (size_t i = 0; i < item->tuple_items.size(); ++i) {
            printItem(item->tuple_items[i]);
            if (i < item->tuple_items.size() - 1) std::cout << ", ";
        }
        std::cout << ")";
    }
}

/**
 * @brief Escapes string characters back to raw escape formatting.
 */
std::string CSEMachine::escapeString(const std::string& s) {
    std::string res;
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length() && s[i+1] == 'n') { res += '\n'; ++i; }
        else if (s[i] == '\\' && i + 1 < s.length() && s[i+1] == 't') { res += '\t'; ++i; }
        else res += s[i];
    }
    return res;
}
