#include "anx.h"
#include "opti.h"

//===---------------------------------------------------------------------===//
// Opti - IR Optimizations for LLVM functions.
//===---------------------------------------------------------------------===//

void opti::optimize(llvm::Function *F)
{
    llvm::EliminateUnreachableBlocks(*F);
}
