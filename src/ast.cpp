#include "anx.h"
#include "lexer.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// AST - This module implements the Abstract Syntax Tree (AST) for Anx
//
// The structure of the AST follows the following hierarchy:
// Node - The base type for AST nodes (defined in ast.h)
//   ProgramNode - The root node of the AST
//   FnDecl - A function declaration
//   VarDecl - A variable declaration
//   ScopeNode - A node consisting of a list of nodes
//   StmtNode - A node that represents a value or evaluates in some way
//     RetStmt - A return statement
//     IfStmt - An if statement
//     BinOpStmt - A binary operation statement
//     UnOpStmt - A unary operation statement
//     AssignStmt - A variable assignment statement
//     CallStmt - A function call statement
//     IdentStmt - A variable statement
//     CastStmt - A typecast statement
//     I32Stmt - An 32bit integer literal statement
//     BoolStmt - A boolean literal statement
//     VoidStmt - A void literal statement
//===---------------------------------------------------------------------===//

std::unique_ptr<ast::StmtNode> parse_stmt();
std::unique_ptr<ast::StmtNode> parse_expr();
std::unique_ptr<ast::StmtNode> parse_primary();

std::unique_ptr<ast::StmtNode> parse_identifier()
{
    std::string name = lex::tok.val;

    lex::eat(); // eat identifier

    if (lex::tok.tok == lex::tok_parens)
    {
        lex::eat(); // eat (

        std::vector<std::unique_ptr<ast::Node>> args;

        while (1)
        {
            args.push_back(parse_expr());

            if (lex::tok.tok == lex::tok_parene)
                break;

            if (lex::tok.tok != lex::tok_comma)
                perr("Expected ',' or ')' in function call");

            lex::eat(); // eat ,
        }

        lex::eat(); // eat )

        return std::make_unique<ast::CallStmt>(std::move(name), std::move(args));
    }

    return std::make_unique<ast::IdentStmt>(std::move(name));
}

std::unique_ptr<ast::ScopeNode> parse_scope()
{
    lex::eat(); // eat {

    std::vector<std::unique_ptr<ast::Node>> nodes;

    while (1)
    {
        switch (lex::tok.tok)
        {
        case lex::tok_curlye:
            lex::eat(); // eat }
            return std::make_unique<ast::ScopeNode>(std::move(nodes));
        case lex::tok_curlys:
            nodes.push_back(parse_scope());
            break;
        default:
            nodes.push_back(parse_stmt());
        }
    }
}

std::unique_ptr<ast::FnDecl> parse_fn()
{
    lex::eat(); // eat fn

    if (lex::tok.tok != lex::tok_identifier)
        perr("Expected identifier after function declaration");

    std::string name = lex::tok.val;

    lex::eat(); // eat identifier

    if (lex::tok.tok != lex::tok_parens)
        perr("Expected '(' after function name in function declaration");

    lex::eat(); // eat (

    std::vector<std::pair<std::string, std::string>> args;

    if (lex::tok.tok != lex::tok_parene)
    {
        while (1)
        {
            if (lex::tok.tok != lex::tok_identifier)
                perr("Expected identifier in function argument list in function declaration, got '" + lex::tok.val + "' instead");

            std::string name = lex::tok.val;

            lex::eat(); // eat name

            if (lex::tok.tok != lex::tok_type)
                perr("Expected ':' after variable name in argument list in function declaration");

            lex::eat(); // eat :

            if (lex::tok.tok != lex::tok_identifier)
                perr("Expected identifier after type in function argument list in function declaration");

            args.push_back(std::make_pair(name, lex::tok.val));

            lex::eat(); // eat type

            if (lex::tok.tok == lex::tok_parene)
                break;

            if (lex::tok.tok != lex::tok_comma)
                perr("Expected ',' or ')' in argument list in function declaration");

            lex::eat(); // eat ,
        }
    }

    lex::eat(); // eat )

    if (lex::tok.tok != lex::tok_identifier)
        perr("Expected return type after function argument list in function declaration");

    std::string type = lex::tok.val;

    lex::eat(); // eat return type

    if (lex::tok.tok == lex::tok_curlys)
        return std::make_unique<ast::FnDecl>(std::move(name), std::move(args), std::move(type), parse_scope());

    return std::make_unique<ast::FnDecl>(std::move(name), std::move(args), std::move(type), parse_stmt());
}

std::unique_ptr<ast::StmtNode> parse_paren_expr()
{
    lex::eat(); // eat (

    std::unique_ptr<ast::StmtNode> expr = parse_expr();

    if (lex::tok.tok != lex::tok_parene)
        perr("Expected ')' to close parenthetical expression");

    lex::eat(); // eat )

    return expr;
}

std::unique_ptr<ast::StmtNode> parse_unary()
{
    std::string op = lex::tok.val;

    lex::eat(); // eat op

    return std::make_unique<ast::UnOpStmt>(std::move(op), parse_primary());
}

std::unique_ptr<ast::I32Stmt> parse_i32()
{
    int val = lex::tok.i32val;

    lex::eat(); // eat integer

    return std::make_unique<ast::I32Stmt>(val);
}

std::unique_ptr<ast::BoolStmt> parse_bool()
{
    bool val = lex::tok.bval;

    lex::eat(); // eat boolean

    return std::make_unique<ast::BoolStmt>(val);
}

std::unique_ptr<ast::StmtNode> parse_primary()
{
    switch (lex::tok.tok)
    {
    case lex::tok_identifier:
        return parse_identifier();
    case lex::tok_integer:
        return parse_i32();
    case lex::tok_boolean:
        return parse_bool();
    case lex::tok_parens:
        return parse_paren_expr();
    case lex::tok_unop:
        return parse_unary();
    default:
        perr("Expected expression, got '" + lex::tok.val + "' instead");
    }

    return nullptr;
}

std::unique_ptr<ast::StmtNode> parse_binop(int priority, std::unique_ptr<ast::StmtNode> lhs)
{
    while (1)
    {
        int c_prio = lex::prio(lex::tok.val);

        if (c_prio < priority)
            return lhs;

        std::string op = lex::tok.val;

        lex::eat(); // eat op

        std::unique_ptr<ast::StmtNode> rhs = parse_primary();

        int next_prio = lex::prio(lex::tok.val);

        if (c_prio < next_prio)
            rhs = parse_binop(c_prio + 1, std::move(rhs));

        lhs = std::make_unique<ast::BinOpStmt>(std::move(op), std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<ast::StmtNode> parse_expr()
{
    std::unique_ptr<ast::StmtNode> lhs = parse_primary();

    if (lex::tok.tok == lex::tok_binop)
        lhs = parse_binop(0, std::move(lhs));

    return lhs;
}

std::unique_ptr<ast::IfStmt> parse_if()
{
    lex::eat(); // eat if

    std::unique_ptr<ast::Node> cond = parse_expr();

    std::unique_ptr<ast::Node> then;

    if (lex::tok.tok == lex::tok_curlys)
        then = parse_scope();
    else
        then = parse_stmt();

    std::unique_ptr<ast::Node> els;

    if (lex::tok.tok == lex::tok_else)
    {
        lex::eat(); // eat else

        if (lex::tok.tok == lex::tok_curlys)
            els = parse_scope();
        else
            els = parse_stmt();
    }

    return std::make_unique<ast::IfStmt>(std::move(cond), std::move(then), std::move(els));
}

std::unique_ptr<ast::RetStmt> parse_ret()
{
    lex::eat(); // eat ret

    std::unique_ptr<ast::RetStmt> ret = std::make_unique<ast::RetStmt>(parse_expr());

    if (lex::tok.tok != lex::tok_eol)
        perr("expected semicolon at end of return statement");

    lex::eat(); // eat ;

    return ret;
}

std::unique_ptr<ast::StmtNode> parse_stmt()
{
    switch (lex::tok.tok)
    {
    case lex::tok_if:
        return parse_if();
    case lex::tok_identifier:
        return parse_identifier();
    case lex::tok_ret:
        return parse_ret();
    default:
        perr("Unexpected token '" + lex::tok.val + "'");
    }

    return nullptr;
}

// Generate the program AST
std::unique_ptr<ast::ProgramNode> ast::gen_ast()
{
    lex::eat(); // generate the first token

    std::vector<std::unique_ptr<FnDecl>> decls;

    while (1)
    {
        switch (lex::tok.tok)
        {
        case lex::tok_eof:
            return std::make_unique<ProgramNode>(std::move(decls));
        case lex::tok_fn:
            decls.push_back(parse_fn());
            break;
        default:
            perr("Only declarations are permitted at the top level, got '" + lex::tok.val + "' instead");
        }
    }
}
