#include "anx.h"
#include "ir.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// IR - This module generates LLVM IR from the Anx AST.
//===---------------------------------------------------------------------===//

std::unique_ptr<llvm::LLVMContext> ir::ctx;
std::unique_ptr<llvm::Module> ir::mod;
std::unique_ptr<llvm::IRBuilder<>> ir::builder;
std::vector<std::map<std::string, ir::Symbol>> ir::symbols;

llvm::Value *ast::ProgramNode::codegen()
{
    ir::symbols.push_back(std::map<std::string, ir::Symbol>());

    for (auto &fn : decls)
        fn->declare();

    for (auto &fn : decls)
        fn->codegen();

    llvm::Value *mainFn = ir::mod->getFunction("main");
    if (!mainFn)
        perr("No main function defined");

    ir::symbols.pop_back();

    return mainFn;
}

void ast::FnDecl::declare()
{
    if (ir::mod->getFunction(name))
        perr("No two functions can have the same name: '" + name + "'");

    std::vector<llvm::Type *> Params(args.size());

    for (unsigned i = 0, e = args.size(); i != e; ++i)
        Params[i] = ir::get_type(args[i].second);

    llvm::FunctionType *FT =
        llvm::FunctionType::get(ir::get_type(type, true), Params, false);

    llvm::Function::LinkageTypes linkage = (is_pub || name == "main") ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;

    F = llvm::Function::Create(FT, linkage, name, ir::mod.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args[Idx++].first);

    ir::symbols.back().insert(std::make_pair(name, ir::Symbol(F)));
}

llvm::Value *ast::FnDecl::codegen()
{
    if (!body)
        return nullptr;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "entry", F);
    ir::builder->SetInsertPoint(BB);

    ir::symbols.push_back(std::map<std::string, ir::Symbol>());

    for (auto &Arg : F->args())
        ir::symbols.back().insert(std::make_pair(std::string(Arg.getName()), ir::Symbol(&Arg)));

    body->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
    {
        if (type == "void")
            ir::builder->CreateRetVoid();
        else
            perr("Expected return statement at end of function '" + name + "'");
    }

    llvm::EliminateUnreachableBlocks(*F);

    llvm::verifyFunction(*F, &llvm::errs());

    if (verbose)
    {
        llvm::errs() << '\n';
        F->print(llvm::errs());
    }

    ir::symbols.pop_back();

    return F;
}

llvm::Value *ast::IfNode::codegen()
{
    llvm::Value *CondV = cond->codegen();

    llvm::Function *F = ir::builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *ThenBB =
        llvm::BasicBlock::Create(*ir::ctx, "then", F);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*ir::ctx, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*ir::ctx, "ifcont");

    ir::builder->CreateCondBr(CondV, ThenBB, ElseBB);

    ir::builder->SetInsertPoint(ThenBB);

    then->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
        ir::builder->CreateBr(MergeBB);

    F->getBasicBlockList().push_back(ElseBB);
    ir::builder->SetInsertPoint(ElseBB);

    if (els)
        els->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
        ir::builder->CreateBr(MergeBB);

    F->getBasicBlockList().push_back(MergeBB);
    ir::builder->SetInsertPoint(MergeBB);

    return nullptr;
}

llvm::Value *ast::NumStmt::codegen()
{
    if (is_float)
        return llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(llvm::APFloatBase::IEEEsingle(), value));

    return llvm::ConstantInt::get(*ir::ctx, llvm::APSInt(llvm::APInt(width, value, radix), is_unsigned));
}

llvm::Value *ast::RetNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        throw std::runtime_error("Block already has terminator");

    if (!value)
        return ir::builder->CreateRetVoid();

    return ir::builder->CreateRet(value->codegen());
}

llvm::Value *ast::CallStmt::codegen()
{
    ir::Symbol sym = ir::search(name);

    if (sym.kind != sym.sym_fn)
        perr("Attempted to call a non-function '" + name + "'");

    llvm::Function *CalleeF = sym.function;

    if (CalleeF->arg_size() != args.size())
        perr("Incorrect # arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
        ArgsV.push_back(args[i]->codegen());

    if (CalleeF->getReturnType()->isVoidTy())
        return ir::builder->CreateCall(CalleeF, ArgsV);

    return ir::builder->CreateCall(CalleeF, ArgsV, "call");
}

llvm::Value *ast::UnOpStmt::codegen()
{
    llvm::Value *V = val->codegen();

    if (op == "!")
        return ir::builder->CreateNot(V, "not");

    throw std::runtime_error("Invalid unary operator '" + op + "' used");
}

llvm::Value *ast::BinOpStmt::codegen()
{
    llvm::Value *L = lhs->codegen();
    llvm::Value *R = rhs->codegen();

    if (op == "+")
        return ir::builder->CreateAdd(L, R, "add");
    else if (op == "-")
        return ir::builder->CreateSub(L, R, "sub");
    else if (op == "*")
        return ir::builder->CreateMul(L, R, "mul");
    else if (op == "/")
        return ir::builder->CreateSDiv(L, R, "div");
    else if (op == "%")
        return ir::builder->CreateSRem(L, R, "mod");
    else if (op == "<")
        return ir::builder->CreateICmpULT(L, R, "cmp");
    else if (op == ">")
        return ir::builder->CreateICmpUGT(L, R, "cmp");
    else if (op == "<=")
        return ir::builder->CreateICmpULE(L, R, "cmp");
    else if (op == ">=")
        return ir::builder->CreateICmpUGE(L, R, "cmp");
    else if (op == "==")
        return ir::builder->CreateICmpEQ(L, R, "cmp");
    else if (op == "!=")
        return ir::builder->CreateICmpNE(L, R, "cmp");

    throw std::runtime_error("Invalid binary operator '" + op + "' used");
}

llvm::Value *ast::IdentStmt::codegen()
{
    return ir::search(name).value;
}

llvm::Value *ast::ScopeNode::codegen()
{
    if (nodes.empty())
        perr("Cannot have an empty scope!");

    ir::symbols.push_back(std::map<std::string, ir::Symbol>());

    llvm::Function *F = ir::builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "scope", F);
    ir::builder->CreateBr(BB);
    ir::builder->SetInsertPoint(BB);

    for (auto &stmt : nodes)
        stmt->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
    {
        llvm::BasicBlock *BB2 = llvm::BasicBlock::Create(*ir::ctx, "scope", F);
        ir::builder->CreateBr(BB2);
        ir::builder->SetInsertPoint(BB2);
    }

    ir::symbols.pop_back();

    return BB;
}
