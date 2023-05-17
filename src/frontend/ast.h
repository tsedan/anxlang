#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../anx.h"
#include "../codegen/ir.h"
#include "../utils.h"
#include "lexer.h"

namespace ast {
class Node {
public:
  virtual ~Node() {}
  virtual ir::Symbol codegen() = 0;
};

class StmtNode : public Node {
public:
  anx::Pos s;
  size_t ssize;
};

class FnDecl : public Node {
public:
  std::string name;
  ty::Type type;
  std::vector<std::string> args;
  std::vector<ty::Type> types;
  std::unique_ptr<Node> body;
  bool is_pub;
  llvm::Function *F;
  anx::Pos d, n, e;

  FnDecl(std::string name, ty::Type type, std::vector<std::string> args,
         std::vector<ty::Type> types, std::unique_ptr<Node> body, bool is_pub,
         anx::Pos d, anx::Pos n, anx::Pos e)
      : name(name), type(std::move(type)), args(std::move(args)),
        types(std::move(types)), body(std::move(body)), is_pub(is_pub), d(d),
        n(n), e(e) {}
  void declare();
  ir::Symbol codegen();
};

class ProgramNode : public Node {
public:
  std::vector<std::unique_ptr<FnDecl>> decls;

  ProgramNode(std::vector<std::unique_ptr<FnDecl>> decls)
      : decls(std::move(decls)) {}
  ir::Symbol codegen();
};

class VarDecl : public Node {
public:
  std::vector<std::string> names;
  std::vector<ty::Type> types;
  std::vector<std::unique_ptr<StmtNode>> inits;
  std::vector<anx::Pos> n;
  anx::Pos d;

  VarDecl(std::vector<std::string> names, std::vector<ty::Type> types,
          std::vector<std::unique_ptr<StmtNode>> inits, std::vector<anx::Pos> n,
          anx::Pos d)
      : names(std::move(names)), types(std::move(types)),
        inits(std::move(inits)), n(std::move(n)), d(d) {}
  ir::Symbol codegen();
};

class ScopeNode : public Node {
public:
  std::vector<std::unique_ptr<Node>> nodes;

  ScopeNode(std::vector<std::unique_ptr<Node>> nodes)
      : nodes(std::move(nodes)) {}
  ir::Symbol codegen();
};

class RetNode : public Node {
public:
  std::unique_ptr<StmtNode> value;
  anx::Pos d;

  RetNode(std::unique_ptr<StmtNode> value, anx::Pos d)
      : value(std::move(value)), d(d) {}
  ir::Symbol codegen();
};

class BreakNode : public Node {
public:
  anx::Pos d;

  BreakNode(anx::Pos d) : d(d) {}
  ir::Symbol codegen();
};

class ContNode : public Node {
public:
  anx::Pos d;

  ContNode(anx::Pos d) : d(d) {}
  ir::Symbol codegen();
};

class IfNode : public Node {
public:
  std::unique_ptr<StmtNode> cond;
  std::unique_ptr<Node> then;
  std::unique_ptr<Node> els;
  anx::Pos d;

  IfNode(std::unique_ptr<StmtNode> cond, std::unique_ptr<Node> then,
         std::unique_ptr<Node> els, anx::Pos d)
      : cond(std::move(cond)), then(std::move(then)), els(std::move(els)),
        d(d) {}
  ir::Symbol codegen();
};

class WhileNode : public Node {
public:
  std::unique_ptr<StmtNode> cond;
  std::unique_ptr<StmtNode> step;
  std::unique_ptr<Node> body;
  anx::Pos d;

  WhileNode(std::unique_ptr<StmtNode> cond, std::unique_ptr<StmtNode> step,
            std::unique_ptr<Node> body, anx::Pos d)
      : cond(std::move(cond)), step(std::move(step)), body(std::move(body)),
        d(d) {}
  ir::Symbol codegen();
};

class AssignStmt : public StmtNode {
public:
  std::string name;
  std::unique_ptr<StmtNode> value;
  anx::Pos n;

  AssignStmt(std::string name, std::unique_ptr<StmtNode> value, anx::Pos n)
      : name(name), value(std::move(value)), n(n) {}
  ir::Symbol codegen();
};

class SwapStmt : public StmtNode {
public:
  std::vector<std::string> names;
  std::vector<std::unique_ptr<StmtNode>> values;
  anx::Pos d;

  SwapStmt(std::vector<std::string> names,
           std::vector<std::unique_ptr<StmtNode>> values, anx::Pos d)
      : names(names), values(std::move(values)), d(d) {}
  ir::Symbol codegen();
};

class BinOpStmt : public StmtNode {
public:
  std::string op;
  std::unique_ptr<StmtNode> lhs;
  std::unique_ptr<StmtNode> rhs;
  anx::Pos n;

  BinOpStmt(std::string op, std::unique_ptr<StmtNode> lhs,
            std::unique_ptr<StmtNode> rhs, anx::Pos n)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)), n(n) {}
  ir::Symbol codegen();
};

class UnOpStmt : public StmtNode {
public:
  std::string op;
  std::unique_ptr<StmtNode> val;
  anx::Pos n;

  UnOpStmt(std::string op, std::unique_ptr<StmtNode> val, anx::Pos n)
      : op(op), val(std::move(val)), n(n) {}
  ir::Symbol codegen();
};

class CallStmt : public StmtNode {
public:
  std::string name;
  std::vector<std::unique_ptr<StmtNode>> args;
  anx::Pos n;

  CallStmt(std::string name, std::vector<std::unique_ptr<StmtNode>> args,
           anx::Pos n)
      : name(name), args(std::move(args)), n(n) {}
  ir::Symbol codegen();
};

class IdentStmt : public StmtNode {
public:
  std::string name;
  anx::Pos n;

  IdentStmt(std::string name, anx::Pos n) : name(name), n(n) {}
  ir::Symbol codegen();
};

class NumStmt : public StmtNode {
public:
  std::string value;
  anx::Pos n;

  NumStmt(std::string value, anx::Pos n) : value(value), n(n) {}
  ir::Symbol codegen();
};

std::unique_ptr<ProgramNode> unit();
std::unique_ptr<FnDecl> step();
} // namespace ast
