#ifndef TREENODE_H
#define TREENODE_H

#include <string>
#include <vector>
#include <memory>

class TreeNode {
public:
    std::string type;
    std::string value;
    
    std::shared_ptr<TreeNode> child;
    std::shared_ptr<TreeNode> sibling;

    TreeNode(std::string nodeType, std::string nodeValue = "") 
        : type(nodeType), value(nodeValue), child(nullptr), sibling(nullptr) {}

    void setChild(std::shared_ptr<TreeNode> c) { child = c; }
    void setSibling(std::shared_ptr<TreeNode> s) { sibling = s; }
};

#endif // TREENODE_H
