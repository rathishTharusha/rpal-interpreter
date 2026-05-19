#ifndef STANDARDIZER_H
#define STANDARDIZER_H

#include "TreeNode.h"
#include <memory>
#include <vector>
#include <string>

class Standardizer {
private:
    std::shared_ptr<TreeNode> makeNode(const std::string& type, const std::string& value = "");
    std::shared_ptr<TreeNode> createLambda(std::shared_ptr<TreeNode> V, std::shared_ptr<TreeNode> E);
    std::shared_ptr<TreeNode> standardizeNode(std::shared_ptr<TreeNode> node);

public:
    Standardizer() {}
    std::shared_ptr<TreeNode> standardize(std::shared_ptr<TreeNode> root);
};

#endif // STANDARDIZER_H
