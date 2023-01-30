#include <vector>

#include "lexer.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// AST - This module implements the Abstract Syntax Tree (AST) for Anx
//
// The structure of the AST follows the following heirarchy:
// ASTNode - The base type for AST nodes (defined in ast.h)
//   GroupNode - A node that consists of a list of other nodes
//   DeclNode - A node that declares something
//     FnDecl - A function declaration
//     VarDecl - A variable declaration
//   StmtNode - A node that represents a value or evaluates in some way
//     ReturnStmt - A return statement
//     IfStmt - An if statement
//     BinOpStmt - A binary operation statement
//     AssignStmt - A variable assignment statement
//     CallStmt - A function call statement
//     IntStmt - An integer literal statement
//     BoolStmt - A boolean literal statement
//     VoidStmt - A void literal statement
//     IdentifierStmt - An identifier
//     CastStmt - A typecast statement
//===---------------------------------------------------------------------===//

class GroupNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> nodes;

public:
    GroupNode(std::vector<std::unique_ptr<ASTNode>> nodes) : nodes(std::move(nodes)) {}
};

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
