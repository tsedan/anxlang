#pragma once

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
    class Symbol final
    {
    public:
        std::string ty;
        llvm::Value *value;

        Symbol(std::string ty, llvm::Value *value) : ty(ty), value(value) {}
    };

    extern std::unique_ptr<llvm::LLVMContext> ctx;
    extern std::unique_ptr<llvm::Module> mod;
    extern std::unique_ptr<llvm::IRBuilder<>> builder;
}
