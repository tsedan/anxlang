#include <map>

#include "anx.h"
#include "ir.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// IR - This module generates LLVM IR from the Anx AST
//===---------------------------------------------------------------------===//

std::unique_ptr<llvm::LLVMContext> ir::ctx;
std::unique_ptr<llvm::Module> ir::mod;
std::unique_ptr<llvm::IRBuilder<>> ir::builder;

std::map<std::string, llvm::Value *> vars;

llvm::Value *ast::ProgramNode::codegen()
{
    for (auto &fn : decls)
        fn->codegen();

    llvm::Value *mainFn = ir::mod->getFunction("main");
    if (!mainFn)
    {
        perr("No main function defined");
        return nullptr;
    }

    return mainFn;
}

llvm::Value *ast::FnDecl::codegen()
{
    std::vector<llvm::Type *> Params(args.size());

    for (unsigned i = 0, e = args.size(); i != e; ++i)
    {
        if (args[i].second == "i32")
            Params[i] = llvm::Type::getInt32Ty(*ir::ctx);
        else if (args[i].second == "bool")
            Params[i] = llvm::Type::getInt1Ty(*ir::ctx);
        else
        {
            perr("Unknown parameter type '" + args[i].second + "'");
            return nullptr;
        }
    }

    llvm::Type *Result;

    if (type == "i32")
        Result = llvm::Type::getInt32Ty(*ir::ctx);
    else if (type == "bool")
        Result = llvm::Type::getInt1Ty(*ir::ctx);
    else if (type == "void")
        Result = llvm::Type::getVoidTy(*ir::ctx);
    else
    {
        perr("Unknown return type '" + type + "'");
        return nullptr;
    }

    llvm::FunctionType *FT =
        llvm::FunctionType::get(Result, Params, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, ir::mod.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args[Idx++].first);

    if (!F->empty())
        perr("Function '" + name + "' already defined");

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "entry", F);
    ir::builder->SetInsertPoint(BB);

    vars.clear();
    for (auto &Arg : F->args())
        vars[std::string(Arg.getName())] = &Arg;

    body->codegen();

    llvm::verifyFunction(*F);

    return F;
}

llvm::Value *ast::IfStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::I32Stmt::codegen()
{
    return llvm::ConstantInt::get(*ir::ctx, llvm::APInt(32, value, true));
}

llvm::Value *ast::RetStmt::codegen()
{
    return ir::builder->CreateRet(value->codegen());
}

llvm::Value *ast::BoolStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::CallStmt::codegen()
{
    llvm::Function *CalleeF = ir::mod->getFunction(name);
    if (!CalleeF)
        perr("Unknown function referenced");

    if (CalleeF->arg_size() != args.size())
        perr("Incorrect # arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
    {
        ArgsV.push_back(args[i]->codegen());
        if (!ArgsV.back())
            return nullptr;
    }

    return ir::builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value *ast::UnOpStmt::codegen()
{
    return nullptr;
}

llvm::Value *ast::BinOpStmt::codegen()
{
    llvm::Value *L = lhs->codegen();
    llvm::Value *R = rhs->codegen();
    if (!L || !R)
        return nullptr;

    if (op == "+")
    {
        return ir::builder->CreateAdd(L, R, "addtmp");
    }
    else if (op == "-")
    {
        return ir::builder->CreateSub(L, R, "subtmp");
    }
    else if (op == "*")
    {
        return ir::builder->CreateMul(L, R, "multmp");
    }
    else if (op == "<")
    {
        return ir::builder->CreateICmpULT(L, R, "cmptmp");
    }
    else
    {
        perr("Invalid binary operator");
        return nullptr;
    }
}

llvm::Value *ast::IdentStmt::codegen()
{
    llvm::Value *V = vars[name];

    if (!V)
        perr("Unknown variable name '" + name + "'");

    return V;
}

llvm::Value *ast::ScopeNode::codegen()
{
    return nullptr;
}
