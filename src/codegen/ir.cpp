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

std::vector<std::map<std::string, ir::Symbol>> ir::symbols;
std::vector<llvm::BasicBlock *> breaks;
std::vector<llvm::BasicBlock *> conts;
ir::Symbol cf;
std::string cfm;

ir::Symbol ir::search(std::string name, size_t row, size_t col)
{
    std::map<std::string, ir::Symbol>::iterator sym;

    for (auto it = ir::symbols.rbegin(); it != ir::symbols.rend(); ++it)
        if ((sym = it->find(name)) != it->end())
            return sym->second;

    anx::perr("unrecognized symbol", row, col, name.size());
}

ir::Symbol ast::ProgramNode::codegen()
{
    ir::symbols.push_back(std::map<std::string, ir::Symbol>());

    for (auto &fn : decls)
        fn->declare();

    for (auto &fn : decls)
        fn->codegen();

    if (!ir::mod->getFunction("main"))
        anx::perr("no `main()` function defined; there is no program entry point");

    ir::symbols.pop_back();

    return ir::Symbol();
}

void ast::FnDecl::declare()
{
    if (ir::mod->getFunction(name))
        anx::perr("a function with this name already exists", nrow, ncol, name.size());

    llvm::Type *ret = ty::toLLVM(type, true);
    if (name == "main")
    {
        is_pub = true;
        ret = llvm::Type::getInt32Ty(*ir::ctx);
    }

    std::vector<llvm::Type *> Params(args.size());

    for (size_t i = 0, e = args.size(); i != e; ++i)
        Params[i] = ty::toLLVM(types[i], false);

    llvm::FunctionType *FT =
        llvm::FunctionType::get(ret, Params, false);

    llvm::Function::LinkageTypes linkage = is_pub ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;

    F = llvm::Function::Create(FT, linkage, name, ir::mod.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args[Idx++]);

    ir::symbols.back().insert(std::make_pair(name, ir::Symbol(F, type, types)));
}

ir::Symbol ast::FnDecl::codegen()
{
    if (!body)
        return ir::Symbol();

    cf = ir::Symbol(F, type, types);
    cfm = name;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ir::ctx, "entry", F);
    ir::builder->SetInsertPoint(BB);

    ir::symbols.push_back(std::map<std::string, ir::Symbol>());

    int i = 0;
    for (auto &Arg : F->args())
    {
        llvm::IRBuilder<> eb(&F->getEntryBlock(), F->getEntryBlock().begin());
        llvm::AllocaInst *a = eb.CreateAlloca(ty::toLLVM(types[i], false), nullptr, Arg.getName());
        ir::builder->CreateStore(&Arg, a);
        ir::symbols.back().insert(std::make_pair(std::string(Arg.getName()), ir::Symbol(a, types[i])));
        i++;
    }

    body->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
    {
        if (name == "main")
            ir::builder->CreateRet(llvm::ConstantInt::get(*ir::ctx, llvm::APInt(32, 0, true)));
        else if (ty::isVoid(type))
            ir::builder->CreateRetVoid();
        else
            anx::perr("expected return instruction at end of non-void function '" + name + "'", erow, ecol);
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

ir::Symbol ast::VarDecl::codegen()
{
    for (size_t i = 0; i < names.size(); i++)
    {
        std::map<std::string, ir::Symbol>::iterator it;
        if ((it = ir::symbols.back().find(names[i])) != ir::symbols.back().end())
            anx::perr("variable name is already used in this scope", nrows[i], ncols[i], names[i].size());

        ir::Symbol cd;
        if (inits[i])
        {
            cd = inits[i]->codegen();

            if (ty::isVoid(types[i]))
                types[i] = cd.typ();
        }

        llvm::IRBuilder<> eb(&cf.fn()->getEntryBlock(), cf.fn()->getEntryBlock().begin());
        llvm::AllocaInst *a = eb.CreateAlloca(ty::toLLVM(types[i], false), nullptr, names[i]);

        if (inits[i])
            ir::builder->CreateStore(cd.coerce(types[i], inits[i]->srow, inits[i]->scol, inits[i]->ssize).val(), a);

        ir::symbols.back().insert(std::make_pair(names[i], ir::Symbol(a, types[i])));
    }

    return ir::Symbol();
}

ir::Symbol ast::AssignStmt::codegen()
{
    ir::Symbol sym = ir::search(name, nrow, ncol);

    ir::Symbol v = value->codegen().coerce(sym.typ(), value->srow, value->scol, value->ssize);

    ir::builder->CreateStore(v.val(), sym.inst());

    return v;
}

ir::Symbol ast::IfNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("instruction is unreachable", drow, dcol);

    llvm::Value *CondV = cond->codegen().coerce(ty::ty_bool, cond->srow, cond->scol, cond->ssize).val();

    llvm::Function *F = cf.fn();

    llvm::BasicBlock *ThenBB =
        llvm::BasicBlock::Create(*ir::ctx, "then", F);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*ir::ctx, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*ir::ctx, "if.exit");

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

    return ir::Symbol();
}

ir::Symbol ast::BreakNode::codegen()
{
    if (breaks.empty())
        anx::perr("break instruction outside of loop", drow, dcol);

    ir::builder->CreateBr(breaks.back());

    return ir::Symbol();
}

ir::Symbol ast::ContNode::codegen()
{
    if (conts.empty())
        anx::perr("continue instruction outside of loop", drow, dcol);

    ir::builder->CreateBr(conts.back());

    return ir::Symbol();
}

ir::Symbol ast::WhileNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("instruction is unreachable", drow, dcol);

    llvm::Function *F = cf.fn();

    llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(*ir::ctx, "loop.entry", F);
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*ir::ctx, "loop");
    llvm::BasicBlock *StepBB = llvm::BasicBlock::Create(*ir::ctx, "loop.step");
    llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(*ir::ctx, "loop.exit");

    ir::builder->CreateBr(EntryBB);
    ir::builder->SetInsertPoint(EntryBB);

    llvm::Value *CondV = cond->codegen().coerce(ty::ty_bool, cond->srow, cond->scol, cond->ssize).val();
    ir::builder->CreateCondBr(CondV, LoopBB, ExitBB);

    F->getBasicBlockList().push_back(LoopBB);
    ir::builder->SetInsertPoint(LoopBB);

    breaks.push_back(ExitBB);
    conts.push_back(StepBB);

    if (body)
        body->codegen();

    breaks.pop_back();
    conts.pop_back();

    if (!ir::builder->GetInsertBlock()->getTerminator())
        ir::builder->CreateBr(StepBB);

    F->getBasicBlockList().push_back(StepBB);
    ir::builder->SetInsertPoint(StepBB);

    if (step)
        step->codegen();

    if (!ir::builder->GetInsertBlock()->getTerminator())
        ir::builder->CreateBr(EntryBB);

    F->getBasicBlockList().push_back(ExitBB);
    ir::builder->SetInsertPoint(ExitBB);

    return ir::Symbol();
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

    if (width < 32)
        return 32;
    else if (width < 64)
        return 64;
    return 128;
}

ir::Symbol ast::NumStmt::codegen()
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

    ty::Type dtype = ty::ty_void;
    for (size_t i = prsd.size() - 1; i > 0; i--)
    {
        if (prsd[i] == 'i' || prsd[i] == 'u' || (radix == 10 && prsd[i] == 'f'))
        {
            dtype = ty::fromString(prsd.substr(i), false, nrow, ncol + i, prsd.size() - i);
            prsd = prsd.substr(0, i);
            break;
        }
    }

    if (radix != 10)
        prsd = prsd.substr(2);

    prsd.erase(remove(prsd.begin(), prsd.end(), '_'), prsd.end());

    if (!prsd.size())
        anx::perr("number literal has no value", nrow, ncol, value.size());

    ir::Symbol sym;
    if (prsd.find('.') == std::string::npos)
    {
        uint32_t width = min_width(prsd, radix);
        sym = ir::Symbol(llvm::ConstantInt::get(*ir::ctx, llvm::APInt(width, prsd, radix)), ty::fromString("u" + std::to_string(width), false));
    }
    else
    {
        if (ty::isDouble(dtype))
            sym = ir::Symbol(llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(llvm::APFloatBase::IEEEdouble(), prsd)), ty::ty_f64);
        else
            sym = ir::Symbol(llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(llvm::APFloatBase::IEEEsingle(), prsd)), ty::ty_f32);
    }

    if (!ty::isVoid(dtype))
        return sym.coerce(dtype, nrow, ncol, value.size());

    return sym;
}

ir::Symbol ast::RetNode::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("instruction is unreachable", drow, dcol);

    if (!value)
    {
        if (cfm == "main")
            ir::builder->CreateRet(llvm::ConstantInt::get(*ir::ctx, llvm::APInt(32, 0, true)));
        else if (ty::isVoid(cf.typ()))
            return ir::Symbol(ir::builder->CreateRetVoid(), ty::ty_void);
        else
            anx::perr("cannot return void from non-void function", drow, dcol);
    }

    ir::Symbol v = value->codegen().coerce(cf.typ(), value->srow, value->scol, value->ssize);
    return ir::Symbol(ir::builder->CreateRet(v.val()), v.typ());
}

ir::Symbol ast::CallStmt::codegen()
{
    if (ir::builder->GetInsertBlock()->getTerminator())
        anx::perr("instruction is unreachable", nrow, ncol);

    ir::Symbol sym = ir::search(name, nrow, ncol);
    llvm::Function *CalleeF = sym.fn();
    std::vector<ty::Type> atypes = sym.atypes();

    if (CalleeF->arg_size() != args.size())
        anx::perr("expected " + std::to_string(CalleeF->arg_size()) + " argument(s), got " + std::to_string(args.size()) + " instead", nrow, ncol, name.size());

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
        ArgsV.push_back(args[i]->codegen().coerce(atypes[i], args[i]->srow, args[i]->scol, args[i]->ssize).val());

    if (ty::isVoid(sym.typ()))
        return ir::Symbol(ir::builder->CreateCall(CalleeF, ArgsV), ty::ty_void);

    return ir::Symbol(ir::builder->CreateCall(CalleeF, ArgsV, "call"), sym.typ());
}

ir::Symbol ast::UnOpStmt::codegen()
{
    ir::Symbol sym = val->codegen();

    if (ty::isVoid(sym.typ()))
        anx::perr("cannot use void type as operand", val->srow, val->scol, val->ssize);

    if (op == "!")
    {
        ir::Symbol coerced = sym.coerce(ty::ty_bool, val->srow, val->scol, val->ssize);
        return ir::Symbol(ir::builder->CreateNot(coerced.val(), "not"), coerced.typ());
    }
    if (op == "-")
    {
        if (ty::isBool(sym.typ()))
            anx::perr("cannot negate boolean type, use `!` instead", val->srow, val->scol, val->ssize);

        if (ty::isUInt(sym.typ()))
        {
            uint32_t width = ty::width(sym.typ());
            return ir::Symbol(ir::builder->CreateNeg(sym.val(), "neg"), ty::fromString("i" + std::to_string(width), false));
        }

        if (ty::isSingle(sym.typ()) || ty::isDouble(sym.typ()))
            return ir::Symbol(ir::builder->CreateFNeg(sym.val(), "neg"), sym.typ());

        return ir::Symbol(ir::builder->CreateNeg(sym.val(), "neg"), sym.typ());
    }

    anx::perr("invalid unary operator", nrow, ncol, op.size());
}

ir::Symbol ast::IdentStmt::codegen()
{
    ir::Symbol sym = ir::search(name, nrow, ncol);
    llvm::AllocaInst *inst = sym.inst();

    return ir::Symbol(ir::builder->CreateLoad(inst->getAllocatedType(), inst, name.c_str()), sym.typ());
}

ir::Symbol ast::ScopeNode::codegen()
{
    ir::symbols.push_back(std::map<std::string, ir::Symbol>());

    for (auto &stmt : nodes)
        stmt->codegen();

    ir::symbols.pop_back();

    return ir::Symbol();
}

ir::Symbol ast::BinOpStmt::codegen()
{
    ir::Symbol lsym = lhs->codegen();
    ir::Symbol rsym = rhs->codegen();
    ty::Type lt = lsym.typ(), rt = rsym.typ();

    ty::Type dtype;
    if (ty::isVoid(lt))
        anx::perr("cannot use void type as operand", lhs->srow, lhs->scol, lhs->ssize);
    else if (ty::isVoid(rt))
        anx::perr("cannot use void type as operand", rhs->srow, rhs->scol, rhs->ssize);
    else if (ty::isDouble(lt) || ty::isDouble(rt))
        dtype = ty::ty_f64;
    else if (ty::isSingle(lt) || ty::isSingle(rt))
        dtype = ty::ty_f32;
    else if (ty::isSInt(lt) || ty::isSInt(rt))
        dtype = ty::fromString('i' + std::to_string(std::max(ty::width(lt), ty::width(rt))), false);
    else if (ty::isUInt(lt) || ty::isUInt(rt))
        dtype = ty::fromString('u' + std::to_string(std::max(ty::width(lt), ty::width(rt))), false);
    else
        dtype = ty::ty_bool;

    llvm::Value *L = lsym.coerce(dtype, lhs->srow, lhs->scol, lhs->ssize).val();
    llvm::Value *R = rsym.coerce(dtype, lhs->srow, lhs->scol, lhs->ssize).val();

    if (op == "+")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFAdd(L, R, "add"), dtype);
        else if (ty::isSInt(dtype) || ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateAdd(L, R, "add"), dtype);
    }
    else if (op == "-")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFSub(L, R, "sub"), dtype);
        else if (ty::isSInt(dtype) || ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateSub(L, R, "sub"), dtype);
    }
    else if (op == "*")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFMul(L, R, "mul"), dtype);
        else if (ty::isSInt(dtype) || ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateMul(L, R, "mul"), dtype);
    }
    else if (op == "/")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFDiv(L, R, "div"), dtype);
        else if (ty::isSInt(dtype))
            return ir::Symbol(ir::builder->CreateSDiv(L, R, "div"), dtype);
        else if (ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateUDiv(L, R, "div"), dtype);
    }
    else if (op == "%")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFRem(L, R, "rem"), dtype);
        else if (ty::isSInt(dtype))
            return ir::Symbol(ir::builder->CreateSRem(L, R, "rem"), dtype);
        else if (ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateURem(L, R, "rem"), dtype);
    }
    else if (op == "<")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFCmpULT(L, R, "cmp"), ty::ty_bool);
        else if (ty::isSInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpSLT(L, R, "cmp"), ty::ty_bool);
        else if (ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpULT(L, R, "cmp"), ty::ty_bool);
    }
    else if (op == ">")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFCmpUGT(L, R, "cmp"), ty::ty_bool);
        else if (ty::isSInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpSGT(L, R, "cmp"), ty::ty_bool);
        else if (ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpUGT(L, R, "cmp"), ty::ty_bool);
    }
    else if (op == "<=")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFCmpULE(L, R, "cmp"), ty::ty_bool);
        else if (ty::isSInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpSLE(L, R, "cmp"), ty::ty_bool);
        else if (ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpULE(L, R, "cmp"), ty::ty_bool);
    }
    else if (op == ">=")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFCmpUGE(L, R, "cmp"), ty::ty_bool);
        else if (ty::isSInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpSGE(L, R, "cmp"), ty::ty_bool);
        else if (ty::isUInt(dtype))
            return ir::Symbol(ir::builder->CreateICmpUGE(L, R, "cmp"), ty::ty_bool);
    }
    else if (op == "==")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFCmpUEQ(L, R, "cmp"), ty::ty_bool);
        else if (ty::isSInt(dtype) || ty::isUInt(dtype) || ty::isBool(dtype))
            return ir::Symbol(ir::builder->CreateICmpEQ(L, R, "cmp"), ty::ty_bool);
    }
    else if (op == "!=")
    {
        if (ty::isDouble(dtype) || ty::isSingle(dtype))
            return ir::Symbol(ir::builder->CreateFCmpUNE(L, R, "cmp"), ty::ty_bool);
        else if (ty::isSInt(dtype) || ty::isUInt(dtype) || ty::isBool(dtype))
            return ir::Symbol(ir::builder->CreateICmpNE(L, R, "cmp"), ty::ty_bool);
    }
    else
    {
        anx::perr("invalid binary operator", nrow, ncol, op.size());
    }

    anx::perr("operation '" + op + "' does not support '" + ty::toString(lt) + "' and '" + ty::toString(rt) + "' type combination", nrow, ncol, op.size());
}
