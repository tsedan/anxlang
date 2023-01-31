#pragma once

#include <memory>

class ASTNode
{
public:
    virtual ~ASTNode() {}
};

std::unique_ptr<ASTNode> gen_ast();
