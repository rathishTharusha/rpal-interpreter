#include "CSEMachine.h"
#include <stdexcept>
#include <cmath>

CSEMachine::CSEMachine(std::shared_ptr<TreeNode> root) : env_counter(0) {
    PE = std::make_shared<Environment>(env_counter++, nullptr);
    current_env = PE;

    deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
    flatten(root, 0);

    auto env_item = std::make_shared<CSEItem>(ItemType::ENV_MARKER, "0");
    env_item->env_ptr = PE;

    control.push_back(env_item);
    for (auto it = deltas[0].rbegin(); it != deltas[0].rend(); ++it) {
        control.push_back(*it);
    }
    
    stack.push_back(env_item);
}

void CSEMachine::flatten(std::shared_ptr<TreeNode> node, int current_delta_index) {
    if (!node) return;
    
    if (node->type == "lambda") {
        int next_delta = deltas.size();
        deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
        flatten(node->child->sibling, next_delta);
        
        auto item = std::make_shared<CSEItem>(ItemType::LAMBDA);
        item->delta_index = next_delta;
        
        auto var_node = node->child;
        if (var_node->type == ",") {
            auto c = var_node->child;
            while(c) {
                item->bound_vars.push_back(c->value);
                c = c->sibling;
            }
        } else {
            item->bound_var = var_node->value;
        }
        deltas[current_delta_index].push_back(item);
    } else if (node->type == "->") {
        auto B = node->child;
        auto T = B->sibling;
        auto E = T->sibling;
        flatten(B, current_delta_index);
        int delta_then = deltas.size();
        deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
        flatten(T, delta_then);
        int delta_else = deltas.size();
        deltas.push_back(std::vector<std::shared_ptr<CSEItem>>());
        flatten(E, delta_else);
        auto beta = std::make_shared<CSEItem>(ItemType::OPERATOR, "beta");
        beta->bound_vars.push_back(std::to_string(delta_then));
        beta->bound_vars.push_back(std::to_string(delta_else));
        deltas[current_delta_index].push_back(beta);
    } else {
        auto c = node->child;
        while (c) {
            flatten(c, current_delta_index);
            c = c->sibling;
        }
        deltas[current_delta_index].push_back(createItemFromNode(node));
    }
}

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
        item->bound_vars.push_back(std::to_string(count));
        return item;
    }
    if (node->type == "<IDENTIFIER>") {
        std::string v = node->value;
        if (v == "Print" || v == "Stern" || v == "Stem" || v == "Conc" || v == "Order" || v == "Null" || 
            v == "Isinteger" || v == "Isstring" || v == "Istuple" || v == "Istruthvalue" || v == "Isfunction" || v == "ItoS" || v == "Cond" || v == "aug" || v == "Ystar") {
            return std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, v);
        }
        return std::make_shared<CSEItem>(ItemType::IDENTIFIER, v);
    }
    // Operator
    return std::make_shared<CSEItem>(ItemType::OPERATOR, node->type);
}

void CSEMachine::evaluate() {
    while (!control.empty()) {
        auto item = control.back();
        control.pop_back();

        /*
        std::cout << "EVAL: "; printItem(item); std::cout << "\n";
        std::cout << "  Stack: "; if (!stack.empty()) printItem(stack.back()); std::cout << "\n";
        */

        if (item->type == ItemType::INTEGER || item->type == ItemType::STRING || 
            item->type == ItemType::TRUTH_VALUE || item->type == ItemType::DUMMY || 
            item->type == ItemType::NIL || item->type == ItemType::CLOSURE || 
            item->type == ItemType::PRIMITIVE_FUNC || 
            (item->type == ItemType::TUPLE && item->value != "tau") || 
            item->type == ItemType::ETA_CLOSURE) {
            stack.push_back(item);
        }
        else if (item->type == ItemType::IDENTIFIER) {
            auto val = current_env->lookup(item->value);
            if (!val) {
                // Not found, treat as string? Actually RPAL errors if unassigned, but some envs treat unbound as strings. Let's error.
                if (item->value == "Print") stack.push_back(std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Print"));
                else {
                    throw std::runtime_error("Undeclared identifier: " + item->value);
                }
            } else {
                stack.push_back(val);
            }
        }
        else if (item->type == ItemType::LAMBDA) {
            auto closure = std::make_shared<CSEItem>(ItemType::CLOSURE);
            closure->env_ptr = current_env;
            closure->delta_index = item->delta_index;
            closure->bound_var = item->bound_var;
            closure->bound_vars = item->bound_vars;
            stack.push_back(closure);
        }
        else if (item->type == ItemType::GAMMA) {
            if (stack.size() < 2) {
                std::cout << "GAMMA Underflow! Stack contents:\n";
                for (auto& s : stack) printItem(s);
                std::cout << "\nControl contents:\n";
                for (auto& c : control) printItem(c);
                std::cout << "\n";
                throw std::runtime_error("Stack underflow in GAMMA. Stack size: " + std::to_string(stack.size()));
            }
            auto rand = stack.back(); stack.pop_back();
            auto rator = stack.back(); stack.pop_back();

            if (rator->type == ItemType::CLOSURE) {
                auto new_env = std::make_shared<Environment>(env_counter++, rator->env_ptr);
                
                if (rator->bound_vars.size() > 0 && rand->type == ItemType::TUPLE) {
                    for (size_t i = 0; i < rator->bound_vars.size(); ++i) {
                        new_env->bindings[rator->bound_vars[i]] = rand->tuple_items[i];
                    }
                } else {
                    new_env->bindings[rator->bound_var] = rand;
                }
                
                auto env_marker = std::make_shared<CSEItem>(ItemType::ENV_MARKER, std::to_string(new_env->id));
                env_marker->env_ptr = new_env;
                env_marker->previous_env = current_env;
                current_env = new_env;
                control.push_back(env_marker);
                stack.push_back(env_marker);
                
                auto& d = deltas[rator->delta_index];
                for (auto it = d.rbegin(); it != d.rend(); ++it) {
                    control.push_back(*it);
                }
            } else if (rator->type == ItemType::PRIMITIVE_FUNC) {
                if (rator->value == "Ystar") {
                    auto eta = std::make_shared<CSEItem>(ItemType::ETA_CLOSURE);
                    eta->closure_ptr = rand;
                    stack.push_back(eta);
                } else if (rator->value == "Cond") {
                    // Cond B T E
                    // Wait! Cond takes B and returns a partially applied Cond(B)
                    auto partial1 = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Cond1");
                    partial1->tuple_items.push_back(rand); // B
                    stack.push_back(partial1);
                } else if (rator->value == "Cond1") {
                    auto partial2 = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Cond2");
                    partial2->tuple_items.push_back(rator->tuple_items[0]); // B
                    partial2->tuple_items.push_back(rand); // T
                    stack.push_back(partial2);
                } else if (rator->value == "Cond2") {
                    auto B = rator->tuple_items[0];
                    auto T = rator->tuple_items[1];
                    auto E = rand; // E
                    auto to_execute = (B->value == "true") ? T : E;
                    // Apply to_execute to dummy
                    control.push_back(std::make_shared<CSEItem>(ItemType::GAMMA));
                    stack.push_back(to_execute);
                    stack.push_back(std::make_shared<CSEItem>(ItemType::DUMMY));
                } else if (rator->value == "Conc1") {
                    auto res = std::make_shared<CSEItem>(ItemType::STRING, rator->tuple_items[0]->value + rand->value);
                    stack.push_back(res);
                } else if (rator->value == "aug1") {
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
                        throw std::runtime_error("aug on non-tuple");
                    }
                } else {
                    stack.push_back(rand); // Put rand back so applyPrimitive can pop it
                    applyPrimitive(rator->value);
                }
            } else if (rator->type == ItemType::ETA_CLOSURE) {
                control.push_back(std::make_shared<CSEItem>(ItemType::GAMMA));
                control.push_back(rand);
                control.push_back(std::make_shared<CSEItem>(ItemType::GAMMA));
                stack.push_back(rator->closure_ptr);
                stack.push_back(rator);
            } else if (rator->type == ItemType::TUPLE) {
                // Tuple selection
                if (rand->type == ItemType::INTEGER) {
                    int idx = std::stoi(rand->value);
                    stack.push_back(rator->tuple_items[idx - 1]);
                }
            }
        }
        else if (item->type == ItemType::ENV_MARKER) {
            current_env = item->previous_env;
            if (stack.size() < 2) throw std::runtime_error("Stack underflow in ENV_MARKER");
            auto result = stack.back(); stack.pop_back();
            auto marker = stack.back(); stack.pop_back();
            stack.push_back(result);
        }
        else if (item->type == ItemType::TUPLE && item->value == "tau") {
            int n = std::stoi(item->bound_vars[0]);
            if (stack.size() < (size_t)n) throw std::runtime_error("Stack underflow in tau");
            auto tuple = std::make_shared<CSEItem>(ItemType::TUPLE);
            for (int i = 0; i < n; ++i) {
                tuple->tuple_items.insert(tuple->tuple_items.begin(), stack.back());
                stack.pop_back();
            }
            stack.push_back(tuple);
        }
        else if (item->type == ItemType::OPERATOR) {
            if (item->value == "beta") {
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
            std::string res = "";
            for (auto& item : rand->tuple_items) {
                res += item->value;
            }
            stack.push_back(std::make_shared<CSEItem>(ItemType::STRING, res));
        } else {
            auto partial = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "Conc1");
            partial->tuple_items.push_back(rand); 
            stack.push_back(partial);
        }
    } else if (func_name == "aug") {
        auto partial = std::make_shared<CSEItem>(ItemType::PRIMITIVE_FUNC, "aug1");
        partial->tuple_items.push_back(rand); 
        stack.push_back(partial);
    } else if (func_name == "Order") {
        if (rand->type == ItemType::TUPLE) stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(rand->tuple_items.size())));
        else stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, "0"));
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

void CSEMachine::applyUnaryOp(const std::string& op) {
    if (stack.size() < 1) throw std::runtime_error("Stack underflow in applyUnaryOp: " + op);
    auto rand = stack.back(); stack.pop_back();
    if (op == "not") stack.push_back(std::make_shared<CSEItem>(ItemType::TRUTH_VALUE, rand->value == "true" ? "false" : "true"));
    else if (op == "neg") stack.push_back(std::make_shared<CSEItem>(ItemType::INTEGER, std::to_string(-std::stoi(rand->value))));
}

void CSEMachine::printItem(std::shared_ptr<CSEItem> item, bool is_conc) {
    if (item->type == ItemType::INTEGER) std::cout << item->value;
    else if (item->type == ItemType::STRING) std::cout << escapeString(item->value);
    else if (item->type == ItemType::TRUTH_VALUE) std::cout << item->value;
    else if (item->type == ItemType::DUMMY) std::cout << "dummy";
    else if (item->type == ItemType::NIL) std::cout << "nil";
    else if (item->type == ItemType::CLOSURE) {
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

std::string CSEMachine::escapeString(const std::string& s) {
    std::string res;
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length() && s[i+1] == 'n') { res += '\n'; ++i; }
        else if (s[i] == '\\' && i + 1 < s.length() && s[i+1] == 't') { res += '\t'; ++i; }
        else res += s[i];
    }
    return res;
}
