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
    public:
        enum
        {
            sym_fn,
            sym_var,
        } kind;
        union
        {
            llvm::Value *value;
            llvm::Function *function;
        };
        anx::Types type;

        Symbol(llvm::Value *value, anx::Types type) : kind(sym_var), value(value), type(type) {}
        Symbol(llvm::Function *function, anx::Types type) : kind(sym_fn), function(function), type(type) {}
    };

    extern std::ifstream anxf; // The anx input file
    extern bool verbose;       // Whether the compiler should print what it's doing

    void perr(std::string msg);
    Types toType(std::string type);
}
