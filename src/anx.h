#pragma once

#include <fstream>
#include <optional>

#include "llvm/MC/TargetRegistry.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

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
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "utils.h"

namespace anx
{
    extern bool verbose;

    [[noreturn]] void perr(std::string msg);
    [[noreturn]] void perr(std::string msg, size_t r, size_t c, size_t s = 1);

    class Symbol final
    {
    private:
        enum
        {
            sym_empty,
            sym_fn,
            sym_val,
            sym_var
        } kind;
        union
        {
            llvm::Value *value;
            llvm::Function *function;
            llvm::AllocaInst *variable;
        };
        ty::Type type;
        std::vector<ty::Type> types;

    public:
        Symbol(llvm::Value *value, ty::Type type) : kind(sym_val), value(value), type(type) {}
        Symbol(llvm::Function *function, ty::Type type, std::vector<ty::Type> types) : kind(sym_fn), function(function), type(type), types(types) {}
        Symbol(llvm::AllocaInst *variable, ty::Type type) : kind(sym_var), variable(variable), type(type) {}
        Symbol() : kind(sym_empty) {}

        // return a new symbol that has been type-coerced to the desired type
        Symbol coerce(ty::Type toType, size_t r, size_t c, size_t s);

        // get llvm function pointer of a function symbol
        llvm::Function *fn()
        {
            if (kind == sym_fn)
                return function;

            anx::perr("attempted to access a non-function as if it were a function");
        }

        // get llvm value pointer of a value symbol
        llvm::Value *val()
        {
            if (kind == sym_val)
                return value;

            anx::perr("attempted to access a non-value as if it were a value");
        }

        llvm::AllocaInst *inst()
        {
            if (kind == sym_var)
                return variable;

            anx::perr("attempted to access a non-variable as if it were a variable");
        }

        // get anx type of a non-empty symbol
        ty::Type typ()
        {
            if (kind == sym_empty)
                anx::perr("attempted to access the type of an empty symbol");

            return type;
        }

        // get anx types list of a function
        std::vector<ty::Type> atypes()
        {
            if (kind != sym_fn)
                anx::perr("attempted to access types list of a non-function");

            return types;
        }
    };
}
