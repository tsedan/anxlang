#include "ir.h"
#include "opti.h"
#include "../frontend/ast.h"

//===---------------------------------------------------------------------===//
// IR - This module generates LLVM IR from the Anx AST.
//===---------------------------------------------------------------------===//

std::unique_ptr<llvm::LLVMContext> ir::ctx;
std::unique_ptr<llvm::Module> ir::mod;
std::unique_ptr<llvm::IRBuilder<>> ir::builder;
std::unique_ptr<llvm::legacy::FunctionPassManager> ir::fpm;

std::vector<std::map<std::string, anx::Symbol>> ir::symbols;
anx::Symbol cf;

anx::Symbol ir::search(std::string name, size_t row, size_t col)
{
    std::map<std::string, anx::Symbol>::iterator sym;

    for (auto it = ir::symbols.rbegin(); it != ir::symbols.rend(); ++it)
        if ((sym = it->find(name)) != it->end())
            return sym->second;

    anx::perr("unrecognized symbol", row, col, name.size());
}

anx::Symbol ast::ProgramNode::codegen()
{
    ir::symbols.push_back(std::map<std::string, anx::Symbol>());

    for (auto &fn : decls)
        fn->declare();

    for (auto &fn : decls)
        fn->codegen();

    if (!ir::mod->getFunction("main"))
        anx::perr("no `main()` function defined; there is no program entry point");

    ir::symbols.pop_back();

    return anx::Symbol();
}

void ast::FnDecl::declare()
{
    if (ir::mod->getFunction(name))
        anx::perr("a function with this name already exists", nrow, ncol, name.size());

    std::vector<llvm::Type *> Params(args.size());

    for (size_t i = 0, e = args.size(); i != e; ++i)
        Params[i] = anx::getType(types[i], false);

    llvm::FunctionType *FT =
        llvm::FunctionType::get(anx::getType(type, true), Params, false);

    if (name == "main" && !is_pub)
        anx::perr("main function must be public (use `pub` keyword)", drow, dcol);

    llvm::Function::LinkageTypes linkage = is_pub ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;

    F = llvm::Function::Create(FT, linkage, name, ir::mod.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args[Idx++]);

    ir::symbols.back().insert(std::make_pair(name, anx::Symbol(F, type, types)));
}

anx::Symbol ast::FnDecl::codegen()
{
    if (!body)
        return anx::Symbol();

    cf = anx::Symbol(F, type, types);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "entry", F);
    ir::builder->SetInsertPoint(BB);

    ir::symbols.push_back(std::map<std::string, anx::Symbol>());

    int i = 0;
    for (auto &Arg : F->args())
    {
        llvm::IRBuilder<> eb(&F->getEntryBlock(), F->getEntryBlock().begin());
        llvm::AllocaInst *a = eb.CreateAlloca(anx::getType(types[i], false), nullptr, Arg.getName());
        ir::builder->CreateStore(&Arg, a);
        ir::symbols.back().insert(std::make_pair(std::string(Arg.getName()), anx::Symbol(a, types[i])));
        i++;
    }

    body->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
    {
        if (anx::isVoid(type))
            ir::builder->CreateRetVoid();
        else
            anx::perr("expected return statement at end of non-void function '" + name + "'", erow, ecol);
    }

    opti::fun(F);

    llvm::verifyFunction(*F, &llvm::errs());

    if (anx::verbose)
    {
        llvm::errs() << '\n';
        F->print(llvm::errs());
    }

    ir::symbols.pop_back();

    return cf;
}

anx::Symbol ast::VarDecl::codegen()
{
    std::map<std::string, anx::Symbol>::iterator it;
    if ((it = ir::symbols.back().find(name)) != ir::symbols.back().end())
        anx::perr("variable name is already used in this scope", nrow, ncol, name.size());

    anx::Symbol cd;
    if (init)
    {
        cd = init->codegen();

        if (type == anx::ty_void)
            type = cd.ty();
    }

    llvm::IRBuilder<> eb(&cf.fn()->getEntryBlock(), cf.fn()->getEntryBlock().begin());
    llvm::AllocaInst *a = eb.CreateAlloca(anx::getType(type, false), nullptr, name);

    if (init)
        ir::builder->CreateStore(cd.coerce(type, init->srow, init->scol, init->ssize).val(), a);

    anx::Symbol sym(a, type);

    ir::symbols.back().insert(std::make_pair(name, sym));

    return sym;
}

anx::Symbol ast::AssignStmt::codegen()
{
    anx::Symbol sym = ir::search(name, nrow, ncol);

    anx::Symbol v = value->codegen().coerce(sym.ty(), value->srow, value->scol, value->ssize);

    ir::builder->CreateStore(v.val(), sym.inst());

    return v;
}

anx::Symbol ast::IfNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("statement is unreachable", drow, dcol);

    llvm::Value *CondV = cond->codegen().coerce(anx::ty_bool, cond->srow, cond->scol, cond->ssize).val();

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

uint32_t min_width(const std::string &str, uint8_t radix)
{
    __uint128_t num = 0;
    for (char c : str)
    {
        if (c >= '0' && c <= '9')
            num = num * radix + (c - '0');
        else if (c >= 'a' && c <= 'z')
            num = num * radix + (c - 'a' + 10);
        else if (c >= 'A' && c <= 'Z')
            num = num * radix + (c - 'A' + 10);
    }

    uint32_t width = 0;
    while (num)
    {
        num >>= 1;
        width++;
    }

    if (width < 8)
        return 8;
    else if (width < 16)
        return 16;
    else if (width < 32)
        return 32;
    else if (width < 64)
        return 64;
    return 128;
}

anx::Symbol ast::NumStmt::codegen()
{
    std::string prsd = value;

    uint8_t radix = 10;
    if (prsd.size() >= 2 && prsd[0] == '0')
    {
        if (prsd[1] == 'x')
            radix = 16;
        else if (prsd[1] == 'b')
            radix = 2;
        else if (prsd[1] == 'o')
            radix = 8;
    }

    anx::Types dtype = anx::ty_void;
    for (size_t i = prsd.size() - 1; i > 0; i--)
    {
        if (prsd[i] == 'i' || prsd[i] == 'u' || (radix == 10 && prsd[i] == 'f'))
        {
            dtype = anx::toType(prsd.substr(i), false, nrow, ncol + i, prsd.size() - i);
            prsd = prsd.substr(0, i);
            break;
        }
    }

    if (radix != 10)
        prsd = prsd.substr(2);

    prsd.erase(remove(prsd.begin(), prsd.end(), '_'), prsd.end());

    if (!prsd.size())
        anx::perr("number literal has no value", nrow, ncol, value.size());

    anx::Symbol sym;
    if (prsd.find('.') == std::string::npos)
    {
        uint32_t width = min_width(prsd, radix);
        sym = anx::Symbol(llvm::ConstantInt::get(*ir::ctx, llvm::APInt(width, prsd, radix)), anx::toType("u" + std::to_string(width), false));
    }
    else
    {
        if (anx::isDouble(dtype))
            sym = anx::Symbol(llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(llvm::APFloatBase::IEEEdouble(), prsd)), anx::ty_f64);
        else
            sym = anx::Symbol(llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(llvm::APFloatBase::IEEEsingle(), prsd)), anx::ty_f32);
    }

    if (!anx::isVoid(dtype))
        return sym.coerce(dtype, nrow, ncol, value.size());

    return sym;
}

anx::Symbol ast::RetNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("statement is unreachable", drow, dcol);

    if (!value)
    {
        if (anx::isVoid(cf.ty()))
            return anx::Symbol(ir::builder->CreateRetVoid(), anx::ty_void);
        else
            anx::perr("cannot return void from non-void function", drow, dcol);
    }

    anx::Symbol v = value->codegen().coerce(cf.ty(), value->srow, value->scol, value->ssize);
    return anx::Symbol(ir::builder->CreateRet(v.val()), v.ty());
}

anx::Symbol ast::CallStmt::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("statement is unreachable", nrow, ncol);

    anx::Symbol sym = ir::search(name, nrow, ncol);
    llvm::Function *CalleeF = sym.fn();
    std::vector<anx::Types> atypes = sym.atypes();

    if (CalleeF->arg_size() != args.size())
        anx::perr("expected " + std::to_string(CalleeF->arg_size()) + " argument(s), got " + std::to_string(args.size()) + " instead", nrow, ncol, name.size());

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
        ArgsV.push_back(args[i]->codegen().coerce(atypes[i], args[i]->srow, args[i]->scol, args[i]->ssize).val());

    if (anx::isVoid(sym.ty()))
        return anx::Symbol(ir::builder->CreateCall(CalleeF, ArgsV), anx::ty_void);

    return anx::Symbol(ir::builder->CreateCall(CalleeF, ArgsV, "call"), sym.ty());
}

anx::Symbol ast::UnOpStmt::codegen()
{
    anx::Symbol sym = val->codegen();

    if (anx::isVoid(sym.ty()))
        anx::perr("cannot use void type as operand", val->srow, val->scol, val->ssize);

    if (op == "!")
        return anx::Symbol(ir::builder->CreateNot(sym.coerce(anx::ty_bool, val->srow, val->scol, val->ssize).val(), "not"), sym.ty());
    if (op == "-")
    {
        if (anx::isBool(sym.ty()))
            anx::perr("cannot negate boolean type, use `!` instead", val->srow, val->scol, val->ssize);

        if (anx::isUInt(sym.ty()))
        {
            uint32_t width = anx::width(sym.ty());
            return anx::Symbol(ir::builder->CreateNeg(sym.val(), "neg"), anx::toType("i" + std::to_string(width), false));
        }

        if (anx::isSingle(sym.ty()) || anx::isDouble(sym.ty()))
            return anx::Symbol(ir::builder->CreateFNeg(sym.val(), "neg"), sym.ty());

        return anx::Symbol(ir::builder->CreateNeg(sym.val(), "neg"), sym.ty());
    }

    anx::perr("invalid unary operator", nrow, ncol, op.size());
}

anx::Symbol ast::IdentStmt::codegen()
{
    anx::Symbol sym = ir::search(name, nrow, ncol);
    llvm::AllocaInst *inst = sym.inst();

    return anx::Symbol(ir::builder->CreateLoad(inst->getAllocatedType(), inst, name.c_str()), sym.ty());
}

anx::Symbol ast::ScopeNode::codegen()
{
    ir::symbols.push_back(std::map<std::string, anx::Symbol>());

    for (auto &stmt : nodes)
        stmt->codegen();

    ir::symbols.pop_back();

    return anx::Symbol();
}

anx::Symbol ast::BinOpStmt::codegen()
{
    anx::Symbol lsym = lhs->codegen();
    anx::Symbol rsym = rhs->codegen();
    anx::Types lt = lsym.ty(), rt = rsym.ty();

    anx::Types dtype;
    if (anx::isVoid(lt))
        anx::perr("cannot use void type as operand", lhs->srow, lhs->scol, lhs->ssize);
    else if (anx::isVoid(rt))
        anx::perr("cannot use void type as operand", rhs->srow, rhs->scol, rhs->ssize);
    else if (anx::isDouble(lt) || anx::isDouble(rt))
        dtype = anx::ty_f64;
    else if (anx::isSingle(lt) || anx::isSingle(rt))
        dtype = anx::ty_f32;
    else if (anx::isSInt(lt) || anx::isSInt(rt))
        dtype = anx::toType('i' + std::to_string(std::max(anx::width(lt), anx::width(rt))), false);
    else if (anx::isUInt(lt) || anx::isUInt(rt))
        dtype = anx::toType('u' + std::to_string(std::max(anx::width(lt), anx::width(rt))), false);
    else
        dtype = anx::ty_bool;

    llvm::Value *L = lsym.coerce(dtype, lhs->srow, lhs->scol, lhs->ssize).val();
    llvm::Value *R = rsym.coerce(dtype, lhs->srow, lhs->scol, lhs->ssize).val();

    if (op == "+")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFAdd(L, R, "add"), dtype);
        else if (anx::isSInt(dtype) || anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateAdd(L, R, "add"), dtype);
    }
    else if (op == "-")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFSub(L, R, "sub"), dtype);
        else if (anx::isSInt(dtype) || anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateSub(L, R, "sub"), dtype);
    }
    else if (op == "*")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFMul(L, R, "mul"), dtype);
        else if (anx::isSInt(dtype) || anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateMul(L, R, "mul"), dtype);
    }
    else if (op == "/")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFDiv(L, R, "div"), dtype);
        else if (anx::isSInt(dtype))
            return anx::Symbol(ir::builder->CreateSDiv(L, R, "div"), dtype);
        else if (anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateUDiv(L, R, "div"), dtype);
    }
    else if (op == "%")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFRem(L, R, "rem"), dtype);
        else if (anx::isSInt(dtype))
            return anx::Symbol(ir::builder->CreateSRem(L, R, "rem"), dtype);
        else if (anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateURem(L, R, "rem"), dtype);
    }
    else if (op == "<")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFCmpULT(L, R, "cmp"), anx::ty_bool);
        else if (anx::isSInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpSLT(L, R, "cmp"), anx::ty_bool);
        else if (anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpULT(L, R, "cmp"), anx::ty_bool);
    }
    else if (op == ">")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFCmpUGT(L, R, "cmp"), anx::ty_bool);
        else if (anx::isSInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpSGT(L, R, "cmp"), anx::ty_bool);
        else if (anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpUGT(L, R, "cmp"), anx::ty_bool);
    }
    else if (op == "<=")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFCmpULE(L, R, "cmp"), anx::ty_bool);
        else if (anx::isSInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpSLE(L, R, "cmp"), anx::ty_bool);
        else if (anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpULE(L, R, "cmp"), anx::ty_bool);
    }
    else if (op == ">=")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFCmpUGE(L, R, "cmp"), anx::ty_bool);
        else if (anx::isSInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpSGE(L, R, "cmp"), anx::ty_bool);
        else if (anx::isUInt(dtype))
            return anx::Symbol(ir::builder->CreateICmpUGE(L, R, "cmp"), anx::ty_bool);
    }
    else if (op == "==")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFCmpUEQ(L, R, "cmp"), anx::ty_bool);
        else if (anx::isSInt(dtype) || anx::isUInt(dtype) || anx::isBool(dtype))
            return anx::Symbol(ir::builder->CreateICmpEQ(L, R, "cmp"), anx::ty_bool);
    }
    else if (op == "!=")
    {
        if (anx::isDouble(dtype) || anx::isSingle(dtype))
            return anx::Symbol(ir::builder->CreateFCmpUNE(L, R, "cmp"), anx::ty_bool);
        else if (anx::isSInt(dtype) || anx::isUInt(dtype) || anx::isBool(dtype))
            return anx::Symbol(ir::builder->CreateICmpNE(L, R, "cmp"), anx::ty_bool);
    }
    else
    {
        anx::perr("invalid binary operator", nrow, ncol, op.size());
    }

    anx::perr("operation '" + op + "' does not support '" + anx::toString(lt) + "' and '" + anx::toString(rt) + "' type combination", nrow, ncol, op.size());
}