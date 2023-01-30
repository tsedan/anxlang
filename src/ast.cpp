#include <vector>

#include "lexer.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// AST - This module implements the Abstract Syntax Tree (AST) for Anx
//
// The structure of the AST follows the following hierarchy:
// ASTNode - The base type for AST nodes (defined in ast.h)
//   DeclNode - A node that declares something
//     FnDecl - A function declaration
//     VarDecl - A variable declaration
//   StmtNode - A node that represents a value or evaluates in some way
//     GroupStmt - A node consisting of a list of nodes, evaluates to the last
//     RetStmt - A return statement
//     IfStmt - An if statement
//     BinOpStmt - A binary operation statement
//     AssignStmt - A variable assignment statement
//     CallStmt - A function call statement
//     IdentStmt - A variable statement
//     CastStmt - A typecast statement
//     I32Stmt - An 32bit integer literal statement
//     BoolStmt - A boolean literal statement
//     VoidStmt - A void literal statement
//===---------------------------------------------------------------------===//

class DeclNode : public ASTNode
{
};

class FnDecl : public DeclNode
{
    std::string name;
    std::vector<std::pair<std::string, std::string>> args;
    std::unique_ptr<ASTNode> body;

public:
    FnDecl(
        std::string name,
        std::vector<std::pair<std::string, std::string>> args,
        std::unique_ptr<ASTNode> body)
        : name(name),
          args(std::move(args)),
          body(std::move(body)) {}
};

class VarDecl : public DeclNode
{
    std::string name;
    std::string type;

public:
    VarDecl(std::string name, std::string type) : name(name), type(type) {}
};

class StmtNode : public ASTNode
{
};

class GroupStmt : public StmtNode
{
    std::vector<std::unique_ptr<ASTNode>> nodes;

public:
    GroupStmt(std::vector<std::unique_ptr<ASTNode>> nodes) : nodes(std::move(nodes)) {}
};

class RetStmt : public StmtNode
{
    std::unique_ptr<ASTNode> value;

public:
    RetStmt(std::unique_ptr<ASTNode> value) : value(std::move(value)) {}
};

class IfStmt : public StmtNode
{
    std::unique_ptr<ASTNode> cond;
    std::unique_ptr<ASTNode> then;
    std::unique_ptr<ASTNode> els;

public:
    IfStmt(
        std::unique_ptr<ASTNode> cond,
        std::unique_ptr<ASTNode> then,
        std::unique_ptr<ASTNode> els)
        : cond(std::move(cond)),
          then(std::move(then)),
          els(std::move(els)) {}
};

class BinOpStmt : public StmtNode
{
    Token op;
    std::unique_ptr<ASTNode> lhs;
    std::unique_ptr<ASTNode> rhs;

public:
    BinOpStmt(
        Token op,
        std::unique_ptr<ASTNode> lhs,
        std::unique_ptr<ASTNode> rhs)
        : op(op),
          lhs(std::move(lhs)),
          rhs(std::move(rhs)) {}
};

class AssignStmt : public StmtNode
{
    std::string name;
    std::unique_ptr<ASTNode> value;

public:
    AssignStmt(std::string name, std::unique_ptr<ASTNode> value)
        : name(name), value(std::move(value)) {}
};

class CallStmt : public StmtNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

public:
    CallStmt(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : name(name), args(std::move(args)) {}
};

class IdentStmt : public StmtNode
{
    std::string name;

public:
    IdentStmt(std::string name) : name(name) {}
};

class CastStmt : public StmtNode
{
    std::string type;
    std::unique_ptr<ASTNode> value;

public:
    CastStmt(std::string type, std::unique_ptr<ASTNode> value)
        : type(type), value(std::move(value)) {}
};

class I32Stmt : public StmtNode
{
    int32_t value;

public:
    I32Stmt(int32_t value) : value(value) {}
};

class BoolStmt : public StmtNode
{
    bool value;

public:
    BoolStmt(bool value) : value(value) {}
};

class VoidStmt : public StmtNode
{
};

ASTNode gen_ast()
{
    return VoidStmt();
}
