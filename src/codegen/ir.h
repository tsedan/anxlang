#pragma once

#include <map>

#include "../anx.h"

namespace ir
{
    extern std::unique_ptr<llvm::LLVMContext> ctx;
    extern std::unique_ptr<llvm::Module> mod;
    extern std::unique_ptr<llvm::IRBuilder<>> builder;
    extern std::unique_ptr<llvm::legacy::FunctionPassManager> fpm;

    extern std::vector<std::map<std::string, anx::Symbol>> symbols;

    void init();
    anx::Symbol search(std::string name, size_t row, size_t col);
}
