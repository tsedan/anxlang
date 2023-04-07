#pragma once

#include <map>

#include "../anx.h"
#include "../utils.h"

namespace ir {
class Symbol final {
private:
  enum { sym_empty, sym_fn, sym_val, sym_var } kind;
  union {
    llvm::Value *value;
    llvm::Function *function;
    llvm::AllocaInst *variable;
  };
  ty::Type type;
  std::vector<ty::Type> types;

public:
  Symbol(llvm::Value *value, ty::Type type)
      : kind(sym_val), value(value), type(type) {}
  Symbol(llvm::Function *function, ty::Type type, std::vector<ty::Type> types)
      : kind(sym_fn), function(function), type(type), types(types) {}
  Symbol(llvm::AllocaInst *variable, ty::Type type)
      : kind(sym_var), variable(variable), type(type) {}
  Symbol() : kind(sym_empty) {}

  // return a new symbol that has been type-coerced to the desired type
  Symbol coerce(ty::Type toType, anx::Pos pos, size_t s);

  // get llvm function pointer of a function symbol
  llvm::Function *fn() {
    if (kind == sym_fn)
      return function;

    anx::perr("attempted to access a non-function as if it were a function");
  }

  // get llvm value pointer of a value symbol
  llvm::Value *val() {
    if (kind == sym_val)
      return value;

    anx::perr("attempted to access a non-value as if it were a value");
  }

  llvm::AllocaInst *inst() {
    if (kind == sym_var)
      return variable;

    anx::perr("attempted to access a non-variable as if it were a variable");
  }

  // get anx type of a non-empty symbol
  ty::Type typ() {
    if (kind == sym_empty)
      anx::perr("attempted to access the type of an empty symbol");

    return type;
  }

  // get anx types list of a function
  std::vector<ty::Type> atypes() {
    if (kind != sym_fn)
      anx::perr("attempted to access types list of a non-function");

    return types;
  }
};

extern std::unique_ptr<llvm::LLVMContext> ctx;
extern std::unique_ptr<llvm::Module> mod;
extern std::unique_ptr<llvm::IRBuilder<>> builder;

extern std::vector<std::map<std::string, Symbol>> symbols;

void init(std::string name);
Symbol search(std::string name, anx::Pos pos);
void add(std::string name, Symbol sym);
std::string mangle(std::string name);
} // namespace ir
