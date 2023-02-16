#pragma once

#include <map>

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace ir
{
    enum SymbolEnum
    {
        sym_fn,  // function symbol
        sym_var, // var symbol
    };

    class Symbol final
    {
    public:
        SymbolEnum kind;
        llvm::Value *value;
        llvm::Function *function;

        Symbol(SymbolEnum kind, llvm::Value *value) : kind(kind), value(value) {}
        Symbol(llvm::Function *function) : kind(sym_fn), function(function) {}
    };

    extern std::unique_ptr<llvm::LLVMContext> ctx;
    extern std::unique_ptr<llvm::Module> mod;
    extern std::unique_ptr<llvm::IRBuilder<>> builder;
    extern std::vector<std::map<std::string, ir::Symbol>> symbols;

    Symbol search(std::string name);
    llvm::Type *get_type(std::string ty, bool allow_void = false);
    llvm::Value *coerce(llvm::Value *val, llvm::Type *destType, bool is_u = false);
}
