#ifndef TREENODE_H
#define TREENODE_H

#include <string>
#include <vector>
#include <memory>

/**
 * @brief Represents a node in the Abstract Syntax Tree (AST).
 * 
 * To represent an arbitrary n-ary tree node using a binary format,
 * we use the First-Child Next-Sibling (FCNS) representation:
 * - 'child' points to the leftmost child of this node.
 * - 'sibling' points to the sibling immediately to the right of this node.
 */
class TreeNode {
public:
    std::string type;   // Node type (e.g. "let", "lambda", "<IDENTIFIER>")
    std::string value;  // The actual value for terminals (e.g. identifier names, integers)
    
    std::shared_ptr<TreeNode> child;   // Leftmost child node
    std::shared_ptr<TreeNode> sibling; // Sibling node on the right

    TreeNode(std::string nodeType, std::string nodeValue = "") 
        : type(nodeType), value(nodeValue), child(nullptr), sibling(nullptr) {}

    // Utility methods to set pointers during bottom-up tree assembly
    void setChild(std::shared_ptr<TreeNode> c) { child = c; }
    void setSibling(std::shared_ptr<TreeNode> s) { sibling = s; }
};

#endif // TREENODE_H
