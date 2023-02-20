#pragma once

#include <map>

namespace ir
{
    extern std::unique_ptr<llvm::LLVMContext> ctx;
    extern std::unique_ptr<llvm::Module> mod;
    extern std::unique_ptr<llvm::IRBuilder<>> builder;
    extern std::vector<std::map<std::string, anx::Symbol>> symbols;

    anx::Symbol search(std::string name, size_t row, size_t col);
}
