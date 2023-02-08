#include "anx.h"
#include "ir.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// IR - This module parses the AST and generates LLVM IR
//===---------------------------------------------------------------------===//

llvm::Value *ast::ProgramNode::codegen()
{
    return nullptr;
}

llvm::Value *ast::FnDecl::codegen()
{
    return nullptr;
}

llvm::Value *ast::IfStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::I32Stmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::RetStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::BoolStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::CallStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::UnOpStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::BinOpStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::IdentStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::ScopeNode::codegen()
{
    return nullptr;
}
