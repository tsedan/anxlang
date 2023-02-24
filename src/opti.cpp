#include "anx.h"
#include "opti.h"
#include "ir.h"

//===---------------------------------------------------------------------===//
// Opti - IR Optimizations for LLVM functions.
//===---------------------------------------------------------------------===//

void opti::fun(llvm::Function *F)
{
    llvm::EliminateUnreachableBlocks(*F);

    ir::fpm->run(*F);
}
