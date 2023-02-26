#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "../anx.h"
#include "../codegen/ir.h"

namespace ast
{
    class Node
    {
    public:
        virtual ~Node() {}
        virtual void print(int ind) {}
        virtual ir::Symbol codegen() = 0;
    };

    class StmtNode : public Node
    {
    public:
        size_t srow, scol, ssize;
    };

    class FnDecl : public Node
    {
    public:
        std::string name;
        ty::Type type;
        std::vector<std::string> args;
        std::vector<ty::Type> types;
        std::unique_ptr<Node> body;
        bool is_pub;
        llvm::Function *F;
        size_t drow, dcol;
        size_t nrow, ncol;
        size_t erow, ecol;

        FnDecl(
            std::string name,
            ty::Type type,
            std::vector<std::string> args,
            std::vector<ty::Type> types,
            std::unique_ptr<Node> body,
            bool is_pub,
            size_t drow, size_t dcol,
            size_t nrow, size_t ncol,
            size_t erow, size_t ecol)
            : name(name),
              type(std::move(type)),
              args(std::move(args)),
              types(std::move(types)),
              body(std::move(body)),
              is_pub(is_pub),
              drow(drow), dcol(dcol),
              nrow(nrow), ncol(ncol),
              erow(erow), ecol(ecol) {}
        void declare();
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< fn " << name << "( ";
            for (size_t i = 0; i < args.size(); i++)
                std::cout << args[i] << " : " << ty::toString(types[i]) << ", ";
            std::cout << ") " << ty::toString(type);
            std::cout << " >" << '\n';
            if (body)
                body->print(ind + 1);
        }
    };

    class ProgramNode : public Node
    {
    public:
        std::vector<std::unique_ptr<FnDecl>> decls;

        ProgramNode(std::vector<std::unique_ptr<FnDecl>> decls) : decls(std::move(decls)) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            for (auto &decl : decls)
                decl->print(ind);
        }
    };

    class VarDecl : public Node
    {
    public:
        std::string name;
        ty::Type type;
        std::unique_ptr<StmtNode> init;
        size_t nrow, ncol;

        VarDecl(std::string name, ty::Type type, std::unique_ptr<StmtNode> init, size_t nrow, size_t ncol)
            : name(name), type(type), init(std::move(init)), nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< var " << name << " : " << ty::toString(type) << " >" << '\n';
            if (init)
                init->print(ind + 1);
        }
    };

    class ScopeNode : public Node
    {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        ScopeNode(std::vector<std::unique_ptr<Node>> nodes) : nodes(std::move(nodes)) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< scope >" << '\n';
            for (auto &node : nodes)
                node->print(ind + 1);
        }
    };

    class RetNode : public Node
    {
    public:
        std::unique_ptr<StmtNode> value;
        size_t drow, dcol;

        RetNode(std::unique_ptr<StmtNode> value, size_t drow, size_t dcol)
            : value(std::move(value)), drow(drow), dcol(dcol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< ret >" << '\n';
            if (value)
                value->print(ind + 1);
        }
    };

    class IfNode : public Node
    {
    public:
        std::unique_ptr<StmtNode> cond;
        std::unique_ptr<Node> then;
        std::unique_ptr<Node> els;
        size_t drow, dcol;

        IfNode(
            std::unique_ptr<StmtNode> cond,
            std::unique_ptr<Node> then,
            std::unique_ptr<Node> els,
            size_t drow, size_t dcol)
            : cond(std::move(cond)),
              then(std::move(then)),
              els(std::move(els)),
              drow(drow), dcol(dcol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< if >" << '\n';
            cond->print(ind + 1);
            std::cout << std::string(ind, ' ') << "< then >" << '\n';
            then->print(ind + 1);
            if (els)
            {
                std::cout << std::string(ind, ' ') << "< else >" << '\n';
                els->print(ind + 1);
            }
        }
    };

    class AssignStmt : public StmtNode
    {
    public:
        std::string name;
        std::unique_ptr<StmtNode> value;
        size_t nrow, ncol;

        AssignStmt(std::string name, std::unique_ptr<StmtNode> value, size_t nrow, size_t ncol)
            : name(name), value(std::move(value)), nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< assign '" << name << "' >" << '\n';
            value->print(ind + 1);
        }
    };

    class BinOpStmt : public StmtNode
    {
    public:
        std::string op;
        std::unique_ptr<StmtNode> lhs;
        std::unique_ptr<StmtNode> rhs;
        size_t nrow, ncol;

        BinOpStmt(
            std::string op,
            std::unique_ptr<StmtNode> lhs,
            std::unique_ptr<StmtNode> rhs,
            size_t nrow, size_t ncol)
            : op(op),
              lhs(std::move(lhs)),
              rhs(std::move(rhs)),
              nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< binop '" << op << "' >" << '\n';
            lhs->print(ind + 1);
            rhs->print(ind + 1);
        }
    };

    class UnOpStmt : public StmtNode
    {
    public:
        std::string op;
        std::unique_ptr<StmtNode> val;
        size_t nrow, ncol;

        UnOpStmt(
            std::string op,
            std::unique_ptr<StmtNode> val,
            size_t nrow, size_t ncol)
            : op(op),
              val(std::move(val)),
              nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< unop '" << op << "' >" << '\n';
            val->print(ind + 1);
        }
    };

    class CallStmt : public StmtNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<StmtNode>> args;
        size_t nrow, ncol;

        CallStmt(std::string name, std::vector<std::unique_ptr<StmtNode>> args, size_t nrow, size_t ncol)
            : name(name), args(std::move(args)), nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< call '" << name << "' >" << '\n';
            for (auto &arg : args)
                arg->print(ind + 1);
        }
    };

    class IdentStmt : public StmtNode
    {
    public:
        std::string name;
        size_t nrow, ncol;

        IdentStmt(std::string name, size_t nrow, size_t ncol) : name(name), nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< ident '" << name << "' >" << '\n';
        }
    };

    class NumStmt : public StmtNode
    {
    public:
        std::string value;
        size_t nrow, ncol;

        NumStmt(std::string value, size_t nrow, size_t ncol)
            : value(value), nrow(nrow), ncol(ncol) {}
        ir::Symbol codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< number " << value << " >" << '\n';
        }
    };

    extern std::unique_ptr<ast::ProgramNode> prog;

    void gen_ast();
}
