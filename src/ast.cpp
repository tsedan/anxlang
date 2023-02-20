#include "anx.h"
#include "ast.h"
#include "lexer.h"

//===---------------------------------------------------------------------===//
// AST - This module implements the Abstract Syntax Tree (AST) for Anx.
//
// The structure of the AST follows the following hierarchy:
// Node - The base type for AST nodes (defined in ast.h)
//   ProgramNode - The root node of the AST
//   FnDecl - A function declaration
//   VarDecl - A variable declaration
//   ScopeNode - A node consisting of a list of nodes
//   RetNode - A return
//   IfNode - An if/else block
//   AssignNode - A variable assignment
//   StmtNode - A node that evaluates
//     BinOpStmt - A binary operation statement
//     UnOpStmt - A unary operation statement
//     CallStmt - A function call statement
//     IdentStmt - A variable statement
//     NumStmt - A number literal statement
//===---------------------------------------------------------------------===//

std::unique_ptr<ast::ProgramNode> ast::prog;

// Get the priority of an operator, such as + or /
int prio(const std::string &op)
{
    if (op == "*" || op == "/" || op == "%")
        return 2;
    else if (op == "+" || op == "-")
        return 1;
    else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        return 0;

    return -1;
}

std::unique_ptr<ast::Node> parse_inst();
std::unique_ptr<ast::StmtNode> parse_expr();
std::unique_ptr<ast::StmtNode> parse_primary();

std::unique_ptr<ast::StmtNode> parse_identifier()
{
    size_t nrow = lex::cr, ncol = lex::cc;
    std::string name = lex::tok.val;

    lex::eat(); // eat identifier

    if (lex::tok.tok == lex::tok_parens)
    {
        lex::eat(); // eat (

        std::vector<std::unique_ptr<ast::StmtNode>> args;

        if (lex::tok.tok != lex::tok_parene)
        {
            while (1)
            {
                args.push_back(parse_expr());

                if (lex::tok.tok == lex::tok_parene)
                    break;

                lex::exp(lex::tok_comma, "expected ',' or ')' in function call");

                lex::eat(); // eat ,
            }
        }

        lex::eat(); // eat )

        return std::make_unique<ast::CallStmt>(std::move(name), std::move(args), nrow, ncol);
    }

    return std::make_unique<ast::IdentStmt>(std::move(name), nrow, ncol);
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
            nodes.push_back(parse_inst());
        }
    }
}

std::unique_ptr<ast::FnDecl> parse_fn(bool is_pub)
{
    size_t drow = lex::cr, dcol = lex::cc;

    lex::eat(); // eat fn/pub

    lex::exp(lex::tok_identifier, "expected name after function declaration");

    std::string name = lex::tok.val;

    size_t nrow = lex::cr, ncol = lex::cc;

    lex::eat(); // eat identifier

    lex::exp(lex::tok_parens, "expected '(' after function name");

    lex::eat(); // eat (

    std::vector<std::string> args;
    std::vector<anx::Types> types;

    if (lex::tok.tok != lex::tok_parene)
    {
        while (1)
        {
            lex::exp(lex::tok_identifier, "expected parameter name in function argument list");

            std::string name = lex::tok.val;

            lex::eat(); // eat name

            lex::exp(lex::tok_type, "expected ':' after parameter name");

            lex::eat(); // eat :

            lex::exp(lex::tok_identifier, "expected type after parameter name");

            args.push_back(name);
            types.push_back(anx::toType(lex::tok.val, false, lex::cr, lex::cc, lex::tok.val.size()));

            lex::eat(); // eat type

            if (lex::tok.tok == lex::tok_parene)
                break;

            lex::exp(lex::tok_comma, "expected ',' or ')' to continue or close argument list");

            lex::eat(); // eat ,
        }
    }

    lex::eat(); // eat )

    anx::Types type = anx::ty_void;

    if (lex::tok.tok == lex::tok_type)
    {
        lex::eat(); // eat :

        type = anx::toType(lex::tok.val, true, lex::cr, lex::cc, lex::tok.val.size());

        lex::eat(); // eat return type
    }

    std::unique_ptr<ast::Node> body;

    switch (lex::tok.tok)
    {
    case lex::tok_eol:
        lex::eat();
        body = nullptr;
        break;
    case lex::tok_curlys:
        body = parse_scope();
        break;
    default:
        body = parse_inst();
    }

    size_t erow = lex::lr, ecol = lex::lc;

    return std::make_unique<ast::FnDecl>(std::move(name), std::move(type), std::move(args), std::move(types), std::move(body), is_pub, drow, dcol, nrow, ncol, erow, ecol);
}

std::unique_ptr<ast::StmtNode> parse_paren_expr()
{
    lex::eat(); // eat (

    std::unique_ptr<ast::StmtNode> expr = parse_expr();

    lex::exp(lex::tok_parene, "expected a closing parenthesis ')'");

    lex::eat(); // eat )

    return expr;
}

std::unique_ptr<ast::StmtNode> parse_unary()
{
    std::string op = lex::tok.val;
    size_t nrow = lex::cr, ncol = lex::cc;

    lex::eat(); // eat op

    return std::make_unique<ast::UnOpStmt>(std::move(op), parse_primary(), nrow, ncol);
}

std::unique_ptr<ast::NumStmt> parse_num()
{
    std::string val = lex::tok.val;
    size_t nrow = lex::cr, ncol = lex::cc;

    lex::eat(); // eat integer

    return std::make_unique<ast::NumStmt>(val, nrow, ncol);
}

std::unique_ptr<ast::StmtNode> parse_primary()
{
    std::unique_ptr<ast::StmtNode> primary;
    size_t srow = lex::cr, scol = lex::cc;

    switch (lex::tok.tok)
    {
    case lex::tok_identifier:
        primary = parse_identifier();
        break;
    case lex::tok_number:
        primary = parse_num();
        break;
    case lex::tok_parens:
        primary = parse_paren_expr();
        break;
    case lex::tok_unop:
        primary = parse_unary();
        break;
    default:
        anx::perr("expected an expression", lex::lr, lex::lc + lex::ls);
    }

    primary->srow = srow;
    primary->scol = scol;
    primary->ssize = lex::lc + lex::ls - scol;

    return primary;
}

std::unique_ptr<ast::StmtNode> parse_binop(int priority, std::unique_ptr<ast::StmtNode> lhs)
{
    while (1)
    {
        size_t srow = lhs->srow, scol = lhs->scol;

        int c_prio = prio(lex::tok.val);

        if (c_prio < priority)
            return lhs;

        std::string op = lex::tok.val;
        size_t nrow = lex::cr, ncol = lex::cc;

        lex::eat(); // eat op

        std::unique_ptr<ast::StmtNode> rhs = parse_primary();

        if (c_prio < prio(lex::tok.val))
            rhs = parse_binop(c_prio + 1, std::move(rhs));

        lhs = std::make_unique<ast::BinOpStmt>(std::move(op), std::move(lhs), std::move(rhs), nrow, ncol);

        lhs->srow = srow;
        lhs->scol = scol;
        if (lex::lr == srow)
            lhs->ssize = lex::lc + lex::ls - lhs->scol;
        else
            lhs->ssize = anx::file[srow].size() - lhs->scol;
    }
}

std::unique_ptr<ast::StmtNode> parse_expr()
{
    std::unique_ptr<ast::StmtNode> lhs = parse_primary();

    if (lex::tok.tok == lex::tok_binop)
        lhs = parse_binop(0, std::move(lhs));

    return lhs;
}

std::unique_ptr<ast::IfNode> parse_if()
{
    lex::eat(); // eat if

    std::unique_ptr<ast::StmtNode> cond = parse_expr();

    std::unique_ptr<ast::Node> then;

    if (lex::tok.tok == lex::tok_curlys)
        then = parse_scope();
    else
        then = parse_inst();

    std::unique_ptr<ast::Node> els;

    if (lex::tok.tok == lex::tok_else)
    {
        lex::eat(); // eat else

        if (lex::tok.tok == lex::tok_curlys)
            els = parse_scope();
        else
            els = parse_inst();
    }

    return std::make_unique<ast::IfNode>(std::move(cond), std::move(then), std::move(els));
}

std::unique_ptr<ast::RetNode> parse_ret()
{
    size_t drow = lex::cr, dcol = lex::cc;

    lex::eat(); // eat ret

    std::unique_ptr<ast::StmtNode> val = nullptr;

    if (lex::tok.tok != lex::tok_eol)
        val = parse_expr();

    std::unique_ptr<ast::RetNode> ret = std::make_unique<ast::RetNode>(std::move(val), drow, dcol);

    return ret;
}

std::unique_ptr<ast::Node> parse_inst()
{
    std::unique_ptr<ast::Node> n;

    switch (lex::tok.tok)
    {
    case lex::tok_if:
        return parse_if();
    case lex::tok_identifier:
        n = parse_identifier();
        break;
    case lex::tok_ret:
        n = parse_ret();
        break;
    default:
        anx::perr("expected an instruction", lex::cr, lex::cc, lex::tok.val.size());
    }

    lex::exp(lex::tok_eol, "expected ';' after instruction");
    lex::eat(); // eat ;

    return n;
}

// Generate the program AST
void ast::gen_ast()
{
    lex::eat(); // generate the first token

    std::vector<std::unique_ptr<FnDecl>> decls;

    while (1)
    {
        switch (lex::tok.tok)
        {
        case lex::tok_eof:
            prog = std::make_unique<ProgramNode>(std::move(decls));
            return;
        case lex::tok_pub:
            decls.push_back(parse_fn(true));
            break;
        case lex::tok_fn:
            decls.push_back(parse_fn(false));
            break;
        default:
            anx::perr("only declarations permitted at the top level", lex::cr, lex::cc, lex::tok.val.size());
        }
    }
}
