#ifndef STANDARDIZER_H
#define STANDARDIZER_H

#include "TreeNode.h"
#include <memory>
#include <vector>
#include <string>

/**
 * @brief Performs tree-to-tree transformations on the AST (Syntactic Desugaring).
 * 
 * Standardizes higher-level language constructs (like let, where, fcn_form, and, rec, conditional branching)
 * into a minimal, binary tree representation containing only fundamental concepts: lambda, gamma, variables, and constants.
 */
class Standardizer {
private:
    // Helper to dynamically allocate new AST nodes during transformations
    std::shared_ptr<TreeNode> makeNode(const std::string& type, const std::string& value = "");
    
    // Core helper to construct lambda nodes, resolving nested/parameter tupling mapping
    std::shared_ptr<TreeNode> createLambda(std::shared_ptr<TreeNode> V, std::shared_ptr<TreeNode> E);
    
    // Performs bottom-up (post-order) traversal and applies transformation rules
    std::shared_ptr<TreeNode> standardizeNode(std::shared_ptr<TreeNode> node);

public:
    Standardizer() {}
    
    // Entry point. Returns the root of the standardized binary tree
    std::shared_ptr<TreeNode> standardize(std::shared_ptr<TreeNode> root);
};

#endif // STANDARDIZER_H
