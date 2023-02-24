#include "anx.h"
#include "opti.h"
#include "ir.h"

//===---------------------------------------------------------------------===//
// Opti - IR Optimizations for LLVM functions.
//===---------------------------------------------------------------------===//

void ir::init()
{
    ctx = std::make_unique<llvm::LLVMContext>();
    mod = std::make_unique<llvm::Module>("Anx Main", *ctx);
    builder = std::make_unique<llvm::IRBuilder<>>(*ctx);

    fpm = std::make_unique<llvm::legacy::FunctionPassManager>(ir::mod.get());

    fpm->add(llvm::createCFGSimplificationPass());
    fpm->add(llvm::createPromoteMemoryToRegisterPass());
    fpm->add(llvm::createAggressiveInstCombinerPass());
    fpm->add(llvm::createReassociatePass());
    fpm->add(llvm::createGVNPass());
    fpm->add(llvm::createAggressiveDCEPass());
    fpm->add(llvm::createLoadStoreVectorizerPass());
    fpm->add(llvm::createSLPVectorizerPass());
    fpm->add(llvm::createJumpThreadingPass());
    fpm->add(llvm::createSinkingPass());
    fpm->add(llvm::createTailCallEliminationPass());

    fpm->doInitialization();
}

void opti::fun(llvm::Function *F)
{
    llvm::EliminateUnreachableBlocks(*F);

    ir::fpm->run(*F);
}
