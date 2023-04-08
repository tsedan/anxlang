#include "intr.h"

//===---------------------------------------------------------------------===//
// IR - This module handles and implements compiler intrinsic functions.
//===---------------------------------------------------------------------===//

std::map<std::string, ir::Symbol> intr::intrinsics;

ir::Symbol intr::handle(std::string name, anx::Pos pos) {
  std::map<std::string, ir::Symbol>::iterator sym;
  if ((sym = intrinsics.find(name)) != intrinsics.end())
    return sym->second;

  if (name == "@out") {
    llvm::Function *F = ir::mod->getFunction("putchar");
    if (!F) {
      llvm::Type *ret = llvm::Type::getInt32Ty(*ir::ctx);
      std::vector<llvm::Type *> Params = {llvm::Type::getInt32Ty(*ir::ctx)};

      llvm::FunctionType *FT = llvm::FunctionType::get(ret, Params, false);
      F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "putchar",
                                 ir::mod.get());
    }

    ir::Symbol s = ir::Symbol(F, ty::ty_i32, {ty::ty_i32});

    intrinsics.insert(std::make_pair(name, s));

    return s;
  }

  anx::perr("unrecognized intrinsic function", pos);
}
