#include "opti.h"
#include "../anx.h"
#include "ir.h"

//===---------------------------------------------------------------------===//
// Opti - Optimization Passes for LLVM IR.
//===---------------------------------------------------------------------===//

std::unique_ptr<llvm::legacy::FunctionPassManager> fpm;

void opti::init(llvm::Module *mod) {
  fpm = std::make_unique<llvm::legacy::FunctionPassManager>(mod);

  fpm->add(llvm::createCFGSimplificationPass());
  fpm->add(llvm::createPromoteMemoryToRegisterPass());
  fpm->add(llvm::createReassociatePass());
  fpm->add(llvm::createGVNPass());
  fpm->add(llvm::createAggressiveDCEPass());
  fpm->add(llvm::createLoadStoreVectorizerPass());
  fpm->add(llvm::createSLPVectorizerPass());
  fpm->add(llvm::createJumpThreadingPass());
  fpm->add(llvm::createSinkingPass());
  fpm->add(llvm::createTailCallEliminationPass());

  fpm->add(llvm::createLoopSimplifyCFGPass());
  fpm->add(llvm::createLoopDeletionPass());
  fpm->add(llvm::createLoopRotatePass());
  fpm->add(llvm::createLoopDataPrefetchPass());
  fpm->add(llvm::createLoopStrengthReducePass());
  fpm->add(llvm::createLoopInterchangePass());
  fpm->add(llvm::createLoopUnrollPass());
  fpm->add(llvm::createLoopUnrollAndJamPass());
  fpm->add(llvm::createLoopLoadEliminationPass());
  fpm->add(llvm::createLoopFlattenPass());
  fpm->add(llvm::createLoopVectorizePass());

  fpm->doInitialization();
}

void opti::fun(llvm::Function *F) {
  llvm::EliminateUnreachableBlocks(*F);
  fpm->run(*F);
}
