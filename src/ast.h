#pragma once

class ASTNode
{
public:
    virtual ~ASTNode() {}
};

ASTNode gen_ast();
