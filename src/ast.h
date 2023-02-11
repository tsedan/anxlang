#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "llvm/IR/BasicBlock.h"

namespace ast
{
    class Node
    {
    public:
        virtual ~Node() {}
        virtual void print(int ind) {}
        virtual llvm::Value *codegen() = 0;
    };

    class FnDecl : public Node
    {
    public:
        std::string name;
        std::vector<std::pair<std::string, std::string>> args;
        std::string type;
        std::unique_ptr<Node> body;
        bool is_pub;
        llvm::Function *F;

        FnDecl(
            std::string name,
            std::vector<std::pair<std::string, std::string>> args,
            std::string type,
            std::unique_ptr<Node> body,
            bool is_pub)
            : name(name),
              args(std::move(args)),
              type(std::move(type)),
              body(std::move(body)),
              is_pub(is_pub) {}
        void declare();
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< fn " << name << "( ";
            for (auto &arg : args)
                std::cout << arg.first << " : " << arg.second << ", ";
            std::cout << ") " << type;
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
        llvm::Value *codegen();

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
        std::string type;

        VarDecl(std::string name, std::string type) : name(name), type(type) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< var " << name << " : " << type << " >" << '\n';
        }
    };

    class StmtNode : public Node
    {
    };

    class ScopeNode : public StmtNode
    {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        ScopeNode(std::vector<std::unique_ptr<Node>> nodes) : nodes(std::move(nodes)) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< scope >" << '\n';
            for (auto &node : nodes)
                node->print(ind + 1);
        }
    };

    class RetStmt : public StmtNode
    {
    public:
        std::unique_ptr<StmtNode> value;

        RetStmt(std::unique_ptr<StmtNode> value) : value(std::move(value)) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< ret >" << '\n';
            if (value)
                value->print(ind + 1);
        }
    };

    class IfStmt : public StmtNode
    {
    public:
        std::unique_ptr<Node> cond;
        std::unique_ptr<Node> then;
        std::unique_ptr<Node> els;

        IfStmt(
            std::unique_ptr<Node> cond,
            std::unique_ptr<Node> then,
            std::unique_ptr<Node> els)
            : cond(std::move(cond)),
              then(std::move(then)),
              els(std::move(els)) {}
        llvm::Value *codegen();

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

    class BinOpStmt : public StmtNode
    {
    public:
        std::string op;
        std::unique_ptr<Node> lhs;
        std::unique_ptr<Node> rhs;

        BinOpStmt(
            std::string op,
            std::unique_ptr<Node> lhs,
            std::unique_ptr<Node> rhs)
            : op(op),
              lhs(std::move(lhs)),
              rhs(std::move(rhs)) {}
        llvm::Value *codegen();

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
        std::unique_ptr<Node> val;

        UnOpStmt(
            std::string op,
            std::unique_ptr<Node> val)
            : op(op),
              val(std::move(val)) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< unop '" << op << "' >" << '\n';
            val->print(ind + 1);
        }
    };

    class AssignStmt : public StmtNode
    {
    public:
        std::string name;
        std::unique_ptr<Node> value;

        AssignStmt(std::string name, std::unique_ptr<Node> value)
            : name(name), value(std::move(value)) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< assign '" << name << "' >" << '\n';
            value->print(ind + 1);
        }
    };

    class CallStmt : public StmtNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<Node>> args;

        CallStmt(std::string name, std::vector<std::unique_ptr<Node>> args)
            : name(name), args(std::move(args)) {}
        llvm::Value *codegen();

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

        IdentStmt(std::string name) : name(name) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< ident '" << name << "' >" << '\n';
        }
    };

    class CastStmt : public StmtNode
    {
    public:
        std::string type;
        std::unique_ptr<Node> value;

        CastStmt(std::string type, std::unique_ptr<Node> value)
            : type(type), value(std::move(value)) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< cast '" << type << "' >" << '\n';
            value->print(ind + 1);
        }
    };

    class I32Stmt : public StmtNode
    {
    public:
        int32_t value;

        I32Stmt(int32_t value) : value(value) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< i32 " << value << " >" << '\n';
        }
    };

    class BoolStmt : public StmtNode
    {
    public:
        bool value;

        BoolStmt(bool value) : value(value) {}
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< bool " << value << " >" << '\n';
        }
    };

    class VoidStmt : public StmtNode
    {
    public:
        llvm::Value *codegen();

        void print(int ind)
        {
            std::cout << std::string(ind, ' ') << "< void >" << '\n';
        }
    };

    std::unique_ptr<ProgramNode> gen_ast();
}
