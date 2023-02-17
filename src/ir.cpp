#include "anx.h"
#include "ir.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// IR - This module generates LLVM IR from the Anx AST.
//===---------------------------------------------------------------------===//

std::unique_ptr<llvm::LLVMContext> ir::ctx;
std::unique_ptr<llvm::Module> ir::mod;
std::unique_ptr<llvm::IRBuilder<>> ir::builder;
std::vector<std::map<std::string, anx::Symbol>> ir::symbols;

anx::Symbol ir::search(std::string name)
{
    std::map<std::string, anx::Symbol>::iterator sym;

    for (auto it = ir::symbols.rbegin(); it != ir::symbols.rend(); ++it)
        if ((sym = it->find(name)) != it->end())
            return sym->second;

    throw std::runtime_error("Unidentified symbol '" + name + "'");
}

anx::Symbol ast::ProgramNode::codegen()
{
    ir::symbols.push_back(std::map<std::string, anx::Symbol>());

    for (auto &fn : decls)
        fn->declare();

    for (auto &fn : decls)
        fn->codegen();

    llvm::Function *mainFn = ir::mod->getFunction("main");
    if (!mainFn)
        anx::perr("No main function defined");

    ir::symbols.pop_back();

    return anx::Symbol();
}

void ast::FnDecl::declare()
{
    if (ir::mod->getFunction(name))
        anx::perr("No two functions can have the same name: '" + name + "'");

    std::vector<llvm::Type *> Params(args.size());

    for (unsigned i = 0, e = args.size(); i != e; ++i)
        Params[i] = anx::getType(args[i].second);

    llvm::FunctionType *FT =
        llvm::FunctionType::get(anx::getType(type, true), Params, false);

    llvm::Function::LinkageTypes linkage = (is_pub || name == "main") ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;

    F = llvm::Function::Create(FT, linkage, name, ir::mod.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args[Idx++].first);

    ir::symbols.back().insert(std::make_pair(name, anx::Symbol(F, type)));
}

anx::Symbol ast::FnDecl::codegen()
{
    if (!body)
        return anx::Symbol();

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "entry", F);
    ir::builder->SetInsertPoint(BB);

    ir::symbols.push_back(std::map<std::string, anx::Symbol>());

    int i = 0;
    for (auto &Arg : F->args())
        ir::symbols.back().insert(std::make_pair(std::string(Arg.getName()), anx::Symbol(&Arg, args[i++].second)));

    body->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
    {
        if (type == anx::ty_void)
            ir::builder->CreateRetVoid();
        else
            anx::perr("Expected return statement at end of function '" + name + "'");
    }

    llvm::EliminateUnreachableBlocks(*F);

    llvm::verifyFunction(*F, &llvm::errs());

    if (anx::verbose)
    {
        llvm::errs() << '\n';
        F->print(llvm::errs());
    }

    ir::symbols.pop_back();

    return anx::Symbol(F, type);
}

anx::Symbol ast::IfNode::codegen()
{
    llvm::Value *CondV = cond->codegen().val();

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

    return anx::Symbol();
}

anx::Symbol ast::NumStmt::codegen()
{
    if (value.find('.') != std::string::npos)
        return anx::Symbol(llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(llvm::APFloatBase::IEEEsingle(), value)), anx::ty_f32);

    return anx::Symbol(llvm::ConstantInt::get(*ir::ctx, llvm::APSInt(llvm::APInt(32, value, 10), false)), anx::ty_i32);
}

anx::Symbol ast::RetNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        throw std::runtime_error("Block already has terminator");

    if (!value)
        return anx::Symbol(ir::builder->CreateRetVoid(), anx::ty_void);

    anx::Symbol v = value->codegen();
    return anx::Symbol(ir::builder->CreateRet(v.val()), v.ty());
}

anx::Symbol ast::CallStmt::codegen()
{
    anx::Symbol sym = ir::search(name);
    llvm::Function *CalleeF = sym.fn();

    if (CalleeF->arg_size() != args.size())
        anx::perr("Incorrect # arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
        ArgsV.push_back(args[i]->codegen().val());

    if (sym.ty() == anx::ty_void)
        return anx::Symbol(ir::builder->CreateCall(CalleeF, ArgsV), anx::ty_void);

    return anx::Symbol(ir::builder->CreateCall(CalleeF, ArgsV, "call"), sym.ty());
}

anx::Symbol ast::UnOpStmt::codegen()
{
    anx::Symbol sym = val->codegen();
    llvm::Value *V = sym.val();

    if (op == "!")
        return anx::Symbol(ir::builder->CreateNot(V, "not"), sym.ty());

    throw std::runtime_error("Invalid unary operator '" + op + "' used");
}

anx::Symbol ast::BinOpStmt::codegen()
{
    anx::Symbol lsym = lhs->codegen();
    anx::Symbol rsym = rhs->codegen();

    llvm::Value *L = lsym.val();
    llvm::Value *R = rsym.val();

    // todo: fix the types here since this is obviously incorrect
    if (op == "+")
        return anx::Symbol(ir::builder->CreateAdd(L, R, "add"), lsym.ty());
    else if (op == "-")
        return anx::Symbol(ir::builder->CreateSub(L, R, "sub"), lsym.ty());
    else if (op == "*")
        return anx::Symbol(ir::builder->CreateMul(L, R, "mul"), lsym.ty());
    else if (op == "/")
        return anx::Symbol(ir::builder->CreateSDiv(L, R, "div"), lsym.ty());
    else if (op == "%")
        return anx::Symbol(ir::builder->CreateSRem(L, R, "mod"), lsym.ty());
    else if (op == "<")
        return anx::Symbol(ir::builder->CreateICmpULT(L, R, "cmp"), lsym.ty());
    else if (op == ">")
        return anx::Symbol(ir::builder->CreateICmpUGT(L, R, "cmp"), lsym.ty());
    else if (op == "<=")
        return anx::Symbol(ir::builder->CreateICmpULE(L, R, "cmp"), lsym.ty());
    else if (op == ">=")
        return anx::Symbol(ir::builder->CreateICmpUGE(L, R, "cmp"), lsym.ty());
    else if (op == "==")
        return anx::Symbol(ir::builder->CreateICmpEQ(L, R, "cmp"), lsym.ty());
    else if (op == "!=")
        return anx::Symbol(ir::builder->CreateICmpNE(L, R, "cmp"), lsym.ty());

    throw std::runtime_error("Invalid binary operator '" + op + "' used");
}

anx::Symbol ast::IdentStmt::codegen()
{
    return ir::search(name);
}

anx::Symbol ast::ScopeNode::codegen()
{
    if (nodes.empty())
        anx::perr("Cannot have an empty scope!");

    ir::symbols.push_back(std::map<std::string, anx::Symbol>());

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

    return anx::Symbol();
}
