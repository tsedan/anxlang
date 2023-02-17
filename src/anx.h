#pragma once

#include <fstream>
#include <optional>

#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
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
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace anx
{
    enum Types
    {
        ty_void,

        ty_i8,
        ty_i16,
        ty_i32,
        ty_i64,
        ty_i128,

        ty_u8,
        ty_u16,
        ty_u32,
        ty_u64,
        ty_u128,

        ty_f32,
        ty_f64,
    };

    class Symbol final
    {
    private:
        enum
        {
            sym_empty,
            sym_fn,
            sym_val,
        } kind;
        union
        {
            llvm::Value *value;
            llvm::Function *function;
        };
        anx::Types type;

    public:
        Symbol(llvm::Value *value, anx::Types type) : kind(sym_val), value(value), type(type) {}
        Symbol(llvm::Function *function, anx::Types type) : kind(sym_fn), function(function), type(type) {}
        Symbol() : kind(sym_empty) {}

        llvm::Function *fn()
        {
            if (kind == sym_fn)
                return function;

            throw std::runtime_error("Attempted to access a non-function as if it were a function");
        }

        llvm::Value *val()
        {
            if (kind == sym_val)
                return value;

            throw std::runtime_error("Attempted to access a non-value as if it were a value");
        }

        anx::Types ty()
        {
            if (kind == sym_empty)
                throw std::runtime_error("Attempted to access the type of an empty symbol");

            return type;
        }
    };

    extern std::ifstream anxf; // The anx input file
    extern bool verbose;       // Whether the compiler should print what it's doing

    void perr(std::string msg);
    Types toType(std::string type);
}
