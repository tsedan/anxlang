#include "anx.h"
#include "lexer.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// AST - This module implements the Abstract Syntax Tree (AST) for Anx
//
// The structure of the AST follows the following hierarchy:
// ASTNode - The base type for AST nodes (defined in ast.h)
//   ProgramNode - The root node of the AST
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

static size_t ti = 0;

class DeclNode : public ASTNode
{
};

class ProgramNode : public ASTNode
{
public:
    std::vector<std::unique_ptr<DeclNode>> decls;

    ProgramNode(std::vector<std::unique_ptr<DeclNode>> decls) : decls(std::move(decls)) {}
};

class FnDecl : public DeclNode
{
public:
    std::string name;
    std::vector<std::pair<std::string, std::string>> args;
    std::string type;
    std::unique_ptr<ASTNode> body;

    FnDecl(
        std::string name,
        std::vector<std::pair<std::string, std::string>> args,
        std::string type,
        std::unique_ptr<ASTNode> body)
        : name(name),
          args(std::move(args)),
          type(std::move(type)),
          body(std::move(body)) {}
};

class VarDecl : public DeclNode
{
public:
    std::string name;
    std::string type;

    VarDecl(std::string name, std::string type) : name(name), type(type) {}
};

class StmtNode : public ASTNode
{
};

class GroupStmt : public StmtNode
{
public:
    std::vector<std::unique_ptr<ASTNode>> nodes;

    GroupStmt(std::vector<std::unique_ptr<ASTNode>> nodes) : nodes(std::move(nodes)) {}
};

class RetStmt : public StmtNode
{
public:
    std::unique_ptr<ASTNode> value;

    RetStmt(std::unique_ptr<ASTNode> value) : value(std::move(value)) {}
};

class IfStmt : public StmtNode
{
public:
    std::unique_ptr<ASTNode> cond;
    std::unique_ptr<ASTNode> then;
    std::unique_ptr<ASTNode> els;

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
public:
    TokEnum op;
    std::unique_ptr<ASTNode> lhs;
    std::unique_ptr<ASTNode> rhs;

    BinOpStmt(
        TokEnum op,
        std::unique_ptr<ASTNode> lhs,
        std::unique_ptr<ASTNode> rhs)
        : op(op),
          lhs(std::move(lhs)),
          rhs(std::move(rhs)) {}
};

class AssignStmt : public StmtNode
{
public:
    std::string name;
    std::unique_ptr<ASTNode> value;

    AssignStmt(std::string name, std::unique_ptr<ASTNode> value)
        : name(name), value(std::move(value)) {}
};

class CallStmt : public StmtNode
{
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    CallStmt(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : name(name), args(std::move(args)) {}
};

class IdentStmt : public StmtNode
{
public:
    std::string name;

    IdentStmt(std::string name) : name(name) {}
};

class CastStmt : public StmtNode
{
public:
    std::string type;
    std::unique_ptr<ASTNode> value;

    CastStmt(std::string type, std::unique_ptr<ASTNode> value)
        : type(type), value(std::move(value)) {}
};

class I32Stmt : public StmtNode
{
public:
    int32_t value;

    I32Stmt(int32_t value) : value(value) {}
};

class BoolStmt : public StmtNode
{
public:
    bool value;

    BoolStmt(bool value) : value(value) {}
};

class VoidStmt : public StmtNode
{
};

std::unique_ptr<ASTNode> parse_any();

std::unique_ptr<GroupStmt> parse_group()
{
    ti++; // eat {

    std::vector<std::unique_ptr<ASTNode>> nodes;

    while (tokens[ti].tok != tok_curlye)
        nodes.push_back(parse_any());

    if (tokens[ti].tok != tok_curlye)
        perr("Expected '}' to close group statement, got '" + tokens[ti].val + "' instead");

    ti++; // eat }

    return std::make_unique<GroupStmt>(std::move(nodes));
}

std::unique_ptr<FnDecl> parse_fn()
{
    ti++; // eat fn

    if (tokens[ti].tok != tok_identifier)
        perr("Expected identifier after function declaration");

    std::string name = tokens[ti].val;

    ti++; // eat identifier

    if (tokens[ti].tok != tok_parens)
        perr("Expected '(' after function name in function declaration");

    ti++; // eat (

    std::vector<std::pair<std::string, std::string>> args;

    while (1)
    {
        if (tokens[ti].tok != tok_identifier)
            perr("Expected identifier in function argument list in function declaration, got '" + tokens[ti].val + "' instead");

        std::string name = tokens[ti].val;

        ti++; // eat name

        if (tokens[ti].tok != tok_type)
            perr("Expected ':' after variable name in argument list in function declaration");

        ti++; // eat :

        if (tokens[ti].tok != tok_identifier)
            perr("Expected identifier after type in function argument list in function declaration");

        args.push_back(std::make_pair(name, tokens[ti].val));

        ti++; // eat type

        if (tokens[ti].tok == tok_parene)
            break;

        if (tokens[ti].tok != tok_comma)
            perr("Expected ',' or ')' in argument list in function declaration");

        ti++; // eat ,
    }

    ti++; // eat )

    if (tokens[ti].tok != tok_identifier)
        perr("Expected return type after function argument list in function declaration");

    std::string type = tokens[ti].val;

    ti++; // eat return type

    if (tokens[ti].tok != tok_curlys)
        perr("Expected '{' code block after function declaration");

    return std::make_unique<FnDecl>(name, std::move(args), std::move(type), parse_group());
}

std::unique_ptr<VarDecl> parse_var()
{
    return nullptr;
}

std::unique_ptr<StmtNode> parse_stmt()
{
    return nullptr;
}

std::unique_ptr<ASTNode> parse_any()
{
    switch (tokens[ti].tok)
    {
    case tok_eof:
        perr("Unexpected end of file");
    case tok_curlys:
        return parse_group();
    default:
        ti++;
        return nullptr;
    }
}

std::unique_ptr<ASTNode> gen_ast()
{
    std::vector<std::unique_ptr<DeclNode>> decls;

    for (Token t : tokens)
        printf("%s ", t.val.c_str());
    printf("\n");

    for (Token t : tokens)
        printf("%d ", t.tok);
    printf("\n");

    while (1)
    {
        switch (tokens[ti].tok)
        {
        case tok_eof:
            return std::make_unique<ProgramNode>(std::move(decls));
        case tok_fn:
            decls.push_back(parse_fn());
            break;
        case tok_var:
            decls.push_back(parse_var());
            break;
        default:
            perr("Only declarations are permitted at the top level, got '" + tokens[ti].val + "' instead");
        }
    }
}
