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

std::unique_ptr<StmtNode> parse_identifier()
{
    std::string name = tokens.at(ti).val;

    ti++; // eat identifier

    if (tokens.at(ti).tok == tok_parens)
    {
        ti++; // eat (

        std::vector<std::unique_ptr<ASTNode>> args;

        while (1)
        {
            args.push_back(parse_any());

            if (tokens.at(ti).tok == tok_parene)
                break;

            if (tokens.at(ti).tok != tok_comma)
                perr("Expected ',' or ')' in function call");

            ti++; // eat ,
        }

        ti++; // eat )

        return std::make_unique<CallStmt>(std::move(name), std::move(args));
    }

    return std::make_unique<IdentStmt>(std::move(name));
}

std::unique_ptr<GroupStmt> parse_group()
{
    ti++; // eat {

    std::vector<std::unique_ptr<ASTNode>> nodes;

    while (tokens.at(ti).tok != tok_curlye)
        nodes.push_back(parse_any());

    if (tokens.at(ti).tok != tok_curlye)
        perr("Expected '}' to close group statement, got '" + tokens.at(ti).val + "' instead");

    ti++; // eat }

    return std::make_unique<GroupStmt>(std::move(nodes));
}

std::unique_ptr<FnDecl> parse_fn()
{
    ti++; // eat fn

    if (tokens.at(ti).tok != tok_identifier)
        perr("Expected identifier after function declaration");

    std::string name = tokens.at(ti).val;

    ti++; // eat identifier

    if (tokens.at(ti).tok != tok_parens)
        perr("Expected '(' after function name in function declaration");

    ti++; // eat (

    std::vector<std::pair<std::string, std::string>> args;

    while (1)
    {
        if (tokens.at(ti).tok != tok_identifier)
            perr("Expected identifier in function argument list in function declaration, got '" + tokens.at(ti).val + "' instead");

        std::string name = tokens.at(ti).val;

        ti++; // eat name

        if (tokens.at(ti).tok != tok_type)
            perr("Expected ':' after variable name in argument list in function declaration");

        ti++; // eat :

        if (tokens.at(ti).tok != tok_identifier)
            perr("Expected identifier after type in function argument list in function declaration");

        args.push_back(std::make_pair(name, tokens.at(ti).val));

        ti++; // eat type

        if (tokens.at(ti).tok == tok_parene)
            break;

        if (tokens.at(ti).tok != tok_comma)
            perr("Expected ',' or ')' in argument list in function declaration");

        ti++; // eat ,
    }

    ti++; // eat )

    if (tokens.at(ti).tok != tok_identifier)
        perr("Expected return type after function argument list in function declaration");

    std::string type = tokens.at(ti).val;

    ti++; // eat return type

    if (tokens.at(ti).tok != tok_curlys)
        perr("Expected '{' code block after function declaration");

    return std::make_unique<FnDecl>(std::move(name), std::move(args), std::move(type), parse_group());
}

std::unique_ptr<VarDecl> parse_var()
{
    return nullptr;
}

std::unique_ptr<StmtNode> parse_primary()
{
    switch (tokens.at(ti).tok)
    {
    case tok_identifier:
        return parse_identifier();
    case tok_integer:
        return std::make_unique<I32Stmt>(tokens.at(ti++).i32val);
    case tok_boolean:
        return std::make_unique<BoolStmt>(tokens.at(ti++).bval);
    default:
        perr("Expected expression, got '" + tokens.at(ti).val + "' instead");
    }

    return nullptr;
}

std::unique_ptr<StmtNode> parse_binop(int priority, std::unique_ptr<StmtNode> lhs)
{
    while (1)
    {
        int prio = get_priority(tokens.at(ti).val);

        if (prio <= priority)
            return lhs;

        // todo: do me
    }

    return nullptr;
}

std::unique_ptr<ASTNode> parse_expr()
{
    bool has_paren = false;
    if (tokens.at(ti).tok == tok_parens)
    {
        has_paren = true;
        ti++; // eat (
    }

    std::unique_ptr<StmtNode> lhs = parse_primary();

    if (tokens.at(ti).tok == tok_binop)
        lhs = parse_binop(0, std::move(lhs));

    if (has_paren)
    {
        if (tokens.at(ti).tok != tok_parene)
            perr("Expected ')' to close parenthetical expression");

        ti++; // eat )
    }

    return std::move(lhs);
}

std::unique_ptr<IfStmt> parse_if()
{
    ti++; // eat if

    if (tokens.at(ti).tok != tok_parens)
        perr("Expected '(' after if statement");

    std::unique_ptr<ASTNode> cond = parse_expr();

    std::unique_ptr<ASTNode> then = parse_any();

    std::unique_ptr<ASTNode> els;

    if (tokens.at(ti).tok == tok_else)
    {
        ti++; // eat else
        els = parse_any();
    }

    return std::make_unique<IfStmt>(std::move(cond), std::move(then), std::move(els));
}

std::unique_ptr<ASTNode> parse_any()
{
    switch (tokens.at(ti).tok)
    {
    case tok_eof:
        perr("Unexpected end of file");
    case tok_curlys:
        return parse_group();
    case tok_if:
        return parse_if();
    case tok_identifier:
        return parse_identifier();
    default:
        perr("Unexpected token '" + tokens.at(ti).val + "'");
    }

    return nullptr;
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
        switch (tokens.at(ti).tok)
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
            perr("Only declarations are permitted at the top level, got '" + tokens.at(ti).val + "' instead");
        }
    }
}
