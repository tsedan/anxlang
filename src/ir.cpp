#include <map>

#include "anx.h"
#include "ir.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// IR - This module generates LLVM IR from the Anx AST.
//===---------------------------------------------------------------------===//

std::unique_ptr<llvm::LLVMContext> ir::ctx;
std::unique_ptr<llvm::Module> ir::mod;
std::unique_ptr<llvm::IRBuilder<>> ir::builder;

std::vector<std::map<std::string, ir::Symbol>> symbols;

ir::Symbol search(std::string name)
{
    std::__1::map<std::__1::string, ir::Symbol>::iterator sym;

    for (auto it = symbols.rbegin(); it != symbols.rend(); ++it)
        if ((sym = it->find(name)) != it->end())
            return sym->second;

    throw std::runtime_error("Unidentified symbol '" + name + "'");
}

llvm::Type *get_type(std::string ty, bool allow_void = false)
{
    if (ty == "void")
    {
        if (allow_void)
            return llvm::Type::getVoidTy(*ir::ctx);
        else
            perr("Void type not allowed here");
    }
    else if (ty == "f32")
        return llvm::Type::getFloatTy(*ir::ctx);
    else if (ty == "f64")
        return llvm::Type::getDoubleTy(*ir::ctx);
    else if (ty == "i8" || ty == "u8")
        return llvm::Type::getInt8Ty(*ir::ctx);
    else if (ty == "i16" || ty == "u16")
        return llvm::Type::getInt16Ty(*ir::ctx);
    else if (ty == "i32" || ty == "u32")
        return llvm::Type::getInt32Ty(*ir::ctx);
    else if (ty == "i64" || ty == "u64")
        return llvm::Type::getInt64Ty(*ir::ctx);
    else if (ty == "i128" || ty == "u128")
        return llvm::Type::getInt128Ty(*ir::ctx);

    throw std::runtime_error("Unrecognized type '" + ty + "'");
}

llvm::Value *ast::ProgramNode::codegen()
{
    symbols.push_back(std::map<std::string, ir::Symbol>());

    for (auto &fn : decls)
        fn->declare();

    for (auto &fn : decls)
        fn->codegen();

    llvm::Value *mainFn = ir::mod->getFunction("main");
    if (!mainFn)
        perr("No main function defined");

    symbols.pop_back();

    return mainFn;
}

void ast::FnDecl::declare()
{
    if (ir::mod->getFunction(name))
        perr("No two functions can have the same name: '" + name + "'");

    std::vector<llvm::Type *> Params(args.size());

    for (unsigned i = 0, e = args.size(); i != e; ++i)
        Params[i] = get_type(args[i].second);

    llvm::FunctionType *FT =
        llvm::FunctionType::get(get_type(type, true), Params, false);

    llvm::Function::LinkageTypes linkage = (is_pub || name == "main") ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;

    F = llvm::Function::Create(FT, linkage, name, ir::mod.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args[Idx++].first);

    symbols.back().insert(std::make_pair(name, ir::Symbol(F)));
}

llvm::Value *ast::FnDecl::codegen()
{
    if (!body)
        return nullptr;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "entry", F);
    ir::builder->SetInsertPoint(BB);

    symbols.push_back(std::map<std::string, ir::Symbol>());

    for (auto &Arg : F->args())
        symbols.back().insert(std::make_pair(std::string(Arg.getName()), ir::Symbol("var", &Arg)));

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

    symbols.pop_back();

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
    ir::Symbol sym = search(name);

    if (sym.kind != "fn")
        perr("Attempted to call a non-function '" + name + "'");

    llvm::Function *CalleeF = sym.function;

    if (CalleeF->arg_size() != args.size())
        perr("Incorrect # arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
        ArgsV.push_back(args[i]->codegen());

    if (CalleeF->getReturnType()->isVoidTy())
        return ir::builder->CreateCall(CalleeF, ArgsV);

    return ir::builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value *ast::UnOpStmt::codegen()
{
    llvm::Value *V = val->codegen();

    if (op == "!")
        return ir::builder->CreateNot(V, "nottmp");

    throw std::runtime_error("Invalid unary operator '" + op + "' used");
}

llvm::Value *ast::BinOpStmt::codegen()
{
    llvm::Value *L = lhs->codegen();
    llvm::Value *R = rhs->codegen();

    if (op == "+")
        return ir::builder->CreateAdd(L, R, "addtmp");
    else if (op == "-")
        return ir::builder->CreateSub(L, R, "subtmp");
    else if (op == "*")
        return ir::builder->CreateMul(L, R, "multmp");
    else if (op == "/")
        return ir::builder->CreateSDiv(L, R, "divtmp");
    else if (op == "%")
        return ir::builder->CreateSRem(L, R, "modtmp");
    else if (op == "<")
        return ir::builder->CreateICmpULT(L, R, "cmptmp");
    else if (op == ">")
        return ir::builder->CreateICmpUGT(L, R, "cmptmp");
    else if (op == "<=")
        return ir::builder->CreateICmpULE(L, R, "cmptmp");
    else if (op == ">=")
        return ir::builder->CreateICmpUGE(L, R, "cmptmp");
    else if (op == "==")
        return ir::builder->CreateICmpEQ(L, R, "cmptmp");
    else if (op == "!=")
        return ir::builder->CreateICmpNE(L, R, "cmptmp");

    throw std::runtime_error("Invalid binary operator '" + op + "' used");
}

llvm::Value *ast::IdentStmt::codegen()
{
    return search(name).value;
}

llvm::Value *ast::ScopeNode::codegen()
{
    if (nodes.empty())
        perr("Cannot have an empty scope!");

    symbols.push_back(std::map<std::string, ir::Symbol>());

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

    symbols.pop_back();

    return BB;
}
