#include "Standardizer.h"
#include <iostream>

/**
 * @brief Helper utility to create a new tree node.
 */
std::shared_ptr<TreeNode> Standardizer::makeNode(const std::string& type, const std::string& value) {
    return std::make_shared<TreeNode>(type, value);
}

/**
 * @brief Resolves parameter definition bindings for Lambda abstractions.
 * Handles normal variable parameters, curried parameters, and parameter tupling.
 * 
 * For parameter tupling (e.g. lambda (x, y). E):
 * - Introduces a new temporary variable T++
 * - Standardizes the body to: gamma(lambda x. (gamma(lambda y. E) (gamma T++ 2))) (gamma T++ 1)
 */
std::shared_ptr<TreeNode> Standardizer::createLambda(std::shared_ptr<TreeNode> V, std::shared_ptr<TreeNode> E) {
    if (V->type == ",") {
        // Parameter tupling case: construct temporary tuple extraction logic
        auto T = makeNode("<IDENTIFIER>", "T++");
        auto current_body = E;
        
        // Extract variables from the comma list
        std::vector<std::shared_ptr<TreeNode>> vars;
        auto v = V->child;
        while (v) {
            auto next = v->sibling;
            v->setSibling(nullptr);
            vars.push_back(v);
            v = next;
        }
        
        // Link variable extractions bottom-up (x = T++ 1, y = T++ 2)
        for (int i = (int)vars.size() - 1; i >= 0; --i) {
            auto gamma_app = makeNode("gamma");
            auto lambda_inner = makeNode("lambda");
            auto gamma_tuple = makeNode("gamma");
            auto T_ref = makeNode("<IDENTIFIER>", "T++");
            auto index = makeNode("<INTEGER>", std::to_string(i + 1));
            
            gamma_tuple->setChild(T_ref);
            T_ref->setSibling(index);
            
            lambda_inner->setChild(vars[i]);
            vars[i]->setSibling(current_body);
            
            gamma_app->setChild(lambda_inner);
            lambda_inner->setSibling(gamma_tuple);
            
            current_body = gamma_app;
        }
        
        auto final_lambda = makeNode("lambda");
        final_lambda->setChild(T);
        T->setSibling(current_body);
        return final_lambda;
    } else {
        // Normal variable/identifier parameter case
        auto final_lambda = makeNode("lambda");
        final_lambda->setChild(V);
        V->setSibling(E);
        return final_lambda;
    }
}

/**
 * @brief Main entry point. Traverses the tree bottom-up post-order.
 * @return The root of the standardized tree.
 */
std::shared_ptr<TreeNode> Standardizer::standardize(std::shared_ptr<TreeNode> root) {
    if (!root) return nullptr;
    return standardizeNode(root);
}

/**
 * @brief Recursive method implementing post-order Sub-Tree Transformational Grammar.
 * Standardizes each node after its sub-trees have been fully standardized.
 */
std::shared_ptr<TreeNode> Standardizer::standardizeNode(std::shared_ptr<TreeNode> node) {
    if (!node) return nullptr;

    // Standardize children first (post-order traversal)
    if (node->child) {
        node->child = standardizeNode(node->child);
    }
    if (node->sibling) {
        node->sibling = standardizeNode(node->sibling);
    }

    std::string t = node->type;
    std::shared_ptr<TreeNode> result = node;

    // 1. Let-Binding: let ( = X E ) P => gamma ( lambda X P ) E
    if (t == "let") {
        auto eq = node->child;
        if (!eq || eq->type != "=") return node;
        auto X = eq->child;
        auto E = X->sibling;
        auto P = eq->sibling;
        
        auto gamma = makeNode("gamma");
        auto lambda = createLambda(X, P);
        
        gamma->setChild(lambda);
        lambda->setSibling(E);
        
        result = gamma;
    }
    // 2. Where-clause: where P ( = X E ) => gamma ( lambda X P ) E
    else if (t == "where") {
        auto P = node->child;
        auto eq = P->sibling;
        if (!eq || eq->type != "=") return node;
        auto X = eq->child;
        auto E = X->sibling;
        
        P->setSibling(nullptr);
        auto gamma = makeNode("gamma");
        auto lambda = createLambda(X, P);
        
        gamma->setChild(lambda);
        lambda->setSibling(E);
        
        result = gamma;
    }
    // 3. Within-clause: within ( = X1 E1 ) ( = X2 E2 ) => = X2 ( gamma ( lambda X1 E2 ) E1 )
    else if (t == "within") {
        auto eq1 = node->child;
        auto eq2 = eq1->sibling;
        if (!eq1 || !eq2 || eq1->type != "=" || eq2->type != "=") return node;
        auto X1 = eq1->child;
        auto E1 = X1->sibling;
        auto X2 = eq2->child;
        auto E2 = X2->sibling;
        
        E1->setSibling(nullptr);
        E2->setSibling(nullptr);
        
        auto new_eq = makeNode("=");
        auto gamma = makeNode("gamma");
        auto lambda = createLambda(X1, E2);
        
        new_eq->setChild(X2);
        X2->setSibling(gamma);
        gamma->setChild(lambda);
        lambda->setSibling(E1);
        
        result = new_eq;
    }
    // 4. Recursive Definition: rec ( = X E ) => = X ( gamma Ystar ( lambda X E ) )
    else if (t == "rec") {
        auto eq = node->child;
        if (!eq || eq->type != "=") return node;
        auto X = eq->child;
        auto E = X->sibling;
        
        E->setSibling(nullptr);
        
        auto new_eq = makeNode("=");
        auto gamma = makeNode("gamma");
        auto ystar = makeNode("<IDENTIFIER>", "Ystar");
        
        auto X_copy = makeNode(X->type, X->value);
        auto lambda = createLambda(X_copy, E);
        
        new_eq->setChild(X);
        X->setSibling(gamma);
        gamma->setChild(ystar);
        ystar->setSibling(lambda);
        
        result = new_eq;
    }
    // 5. Function Form: fcn_form P V1..Vn = E => = P ( lambda V1 .. ( lambda Vn E ) )
    else if (t == "fcn_form") {
        auto P = node->child;
        auto V = P->sibling;
        
        auto new_eq = makeNode("=");
        new_eq->setChild(P);
        
        std::vector<std::shared_ptr<TreeNode>> vars;
        auto curr_V = V;
        while (curr_V->sibling) {
            auto next = curr_V->sibling;
            curr_V->setSibling(nullptr);
            vars.push_back(curr_V);
            curr_V = next;
        }
        auto E = curr_V;
        
        auto current_body = E;
        for (int i = (int)vars.size() - 1; i >= 0; --i) {
            current_body = createLambda(vars[i], current_body);
        }
        
        P->setSibling(current_body);
        result = new_eq;
    }
    // 6. Infix Operator: @ E1 N E2 => gamma ( gamma N E1 ) E2
    else if (t == "@") {
        auto E1 = node->child;
        auto N = E1->sibling;
        auto E2 = N->sibling;
        
        E1->setSibling(nullptr);
        N->setSibling(nullptr);
        E2->setSibling(nullptr);
        
        auto gamma1 = makeNode("gamma");
        auto gamma2 = makeNode("gamma");
        
        gamma1->setChild(gamma2);
        gamma2->setSibling(E2);
        
        gamma2->setChild(N);
        N->setSibling(E1);
        
        result = gamma1;
    }
    // 7. Simultaneous Definitions: and ( = X1 E1 ) ( = X2 E2 ) => = ( , X1 X2 ) ( tau E1 E2 )
    else if (t == "and") {
        auto new_eq = makeNode("=");
        auto comma = makeNode(",");
        auto tau = makeNode("tau");
        
        new_eq->setChild(comma);
        comma->setSibling(tau);
        
        auto eq = node->child;
        std::shared_ptr<TreeNode> first_X = nullptr, last_X = nullptr;
        std::shared_ptr<TreeNode> first_E = nullptr, last_E = nullptr;
        
        while (eq) {
            auto X = eq->child;
            auto E = X->sibling;
            
            X->setSibling(nullptr);
            E->setSibling(nullptr);
            
            if (!first_X) {
                first_X = X; last_X = X;
                first_E = E; last_E = E;
            } else {
                last_X->setSibling(X); last_X = X;
                last_E->setSibling(E); last_E = E;
            }
            eq = eq->sibling;
        }
        
        comma->setChild(first_X);
        tau->setChild(first_E);
        
        result = new_eq;
    }
    // 8. Infix Operator (Duplicate handler present in original codebase for robustness)
    else if (t == "@") {
        auto E1 = node->child;
        auto N = E1->sibling;
        auto E2 = N->sibling;
        
        auto gamma1 = makeNode("gamma");
        auto gamma2 = makeNode("gamma");
        
        gamma1->setChild(gamma2);
        gamma2->setSibling(E2);
        gamma2->setChild(N);
        N->setSibling(E1);
        
        E1->setSibling(nullptr);
        E2->setSibling(nullptr);
        
        result = gamma1;
    }
    // 9. Conditional Expression: -> B T E => gamma ( gamma ( gamma Cond B ) ( lambda () T ) ) ( lambda () E )
    else if (t == "->") {
        auto B = node->child;
        auto T = B->sibling;
        auto E = T->sibling;
        
        auto gamma1 = makeNode("gamma");
        auto gamma2 = makeNode("gamma");
        auto gamma3 = makeNode("gamma");
        auto cond = makeNode("<IDENTIFIER>", "Cond");
        auto lambda1 = makeNode("lambda");
        auto dummy1 = makeNode("()");
        auto lambda2 = makeNode("lambda");
        auto dummy2 = makeNode("()");
        
        gamma1->setChild(gamma2);
        gamma2->setSibling(lambda2);
        gamma2->setChild(gamma3);
        gamma3->setSibling(lambda1);
        gamma3->setChild(cond);
        cond->setSibling(B);
        B->setSibling(nullptr);
        
        lambda1->setChild(dummy1);
        dummy1->setSibling(T);
        T->setSibling(nullptr);
        
        lambda2->setChild(dummy2);
        dummy2->setSibling(E);
        E->setSibling(nullptr);
        
        result = gamma1;
    }
    // 10. Tuples: tau E1..En => aug list chain (gamma (gamma aug (gamma aug nil E1)) E2)
    else if (t == "tau") {
        std::vector<std::shared_ptr<TreeNode>> children;
        auto c = node->child;
        while (c) {
            auto next = c->sibling;
            c->setSibling(nullptr);
            children.push_back(c);
            c = next;
        }
        
        auto res = makeNode("nil");
        for (size_t i = 0; i < children.size(); ++i) {
            auto gamma1 = makeNode("gamma");
            auto gamma2 = makeNode("gamma");
            auto aug = makeNode("<IDENTIFIER>", "aug");
            
            gamma1->setChild(gamma2);
            gamma2->setSibling(children[i]);
            gamma2->setChild(aug);
            aug->setSibling(res);
            
            res = gamma1;
        }
        result = res;
    }
    // 11. Multi-Parameter / Curried Lambdas: lambda V1..Vn E => lambda V1 ( lambda V2 .. ( lambda Vn E ) )
    else if (t == "lambda") {
        auto child1 = node->child;
        if (!child1) return node;

        if (child1->type == ",") {
            auto E = child1->sibling;
            child1->setSibling(nullptr);
            result = createLambda(child1, E);
        } 
        else if (child1->sibling && child1->sibling->sibling) {
            std::vector<std::shared_ptr<TreeNode>> vars;
            auto curr_V = child1;
            while (curr_V->sibling) {
                auto next = curr_V->sibling;
                curr_V->setSibling(nullptr);
                vars.push_back(curr_V);
                curr_V = next;
            }
            auto E = curr_V;
            
            auto current_body = E;
            for (int i = (int)vars.size() - 1; i >= 0; --i) {
                current_body = createLambda(vars[i], current_body);
            }
            result = current_body;
        }
    }

    if (result != node) {
        result->setSibling(node->sibling);
    }

    return result;
}
