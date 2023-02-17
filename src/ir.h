#pragma once

#include <map>

namespace ir
{
    extern std::unique_ptr<llvm::LLVMContext> ctx;
    extern std::unique_ptr<llvm::Module> mod;
    extern std::unique_ptr<llvm::IRBuilder<>> builder;
    extern std::vector<std::map<std::string, anx::Symbol>> symbols;

    anx::Symbol search(std::string name);
    llvm::Type *get_type(anx::Types ty, bool allow_void = false);
    llvm::Value *coerce(llvm::Value *val, llvm::Type *destType, bool is_u = false);
}
