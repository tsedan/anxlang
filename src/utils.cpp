#include <map>

#include "anx.h"
#include "ir.h"
#include "ast.h"

anx::Symbol ir::search(std::string name)
{
    std::map<std::string, anx::Symbol>::iterator sym;

    for (auto it = ir::symbols.rbegin(); it != ir::symbols.rend(); ++it)
        if ((sym = it->find(name)) != it->end())
            return sym->second;

    throw std::runtime_error("Unidentified symbol '" + name + "'");
}

llvm::Type *ir::get_type(anx::Types ty, bool allow_void)
{
    switch (ty)
    {
    case anx::ty_f32:
        return llvm::Type::getFloatTy(*ir::ctx);
    case anx::ty_f64:
        return llvm::Type::getDoubleTy(*ir::ctx);
    case anx::ty_i8:
    case anx::ty_u8:
        return llvm::Type::getInt8Ty(*ir::ctx);
    case anx::ty_i16:
    case anx::ty_u16:
        return llvm::Type::getInt16Ty(*ir::ctx);
    case anx::ty_i32:
    case anx::ty_u32:
        return llvm::Type::getInt32Ty(*ir::ctx);
    case anx::ty_i64:
    case anx::ty_u64:
        return llvm::Type::getInt64Ty(*ir::ctx);
    case anx::ty_i128:
    case anx::ty_u128:
        return llvm::Type::getInt128Ty(*ir::ctx);
    case anx::ty_void:
        if (allow_void)
            return llvm::Type::getVoidTy(*ir::ctx);
        else
            anx::perr("Void type not allowed here");
    default:
        throw std::runtime_error("Unrecognized type");
    }
}

llvm::Value *ir::coerce(llvm::Value *val, llvm::Type *destType, bool is_u)
{
    llvm::Type *fromType = val->getType();

    if (fromType->isVoidTy())
    {
        if (destType->isVoidTy())
            return val;
    }
    else if (fromType->isFloatTy())
    {
        if (destType->isFloatTy())
        {
            return val;
        }
        else if (destType->isDoubleTy())
        {
            return ir::builder->CreateCast(llvm::Instruction::CastOps::FPExt, val, destType, "cast");
        }
        else if (destType->isIntegerTy())
        {
            if (is_u)
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI, val, destType, "cast");
            }
            else
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI, val, destType, "cast");
            }
        }
    }
    else if (fromType->isDoubleTy())
    {
        if (destType->isDoubleTy())
        {
            return val;
        }
        else if (destType->isFloatTy())
        {
            return ir::builder->CreateCast(llvm::Instruction::CastOps::FPTrunc, val, destType, "cast");
        }
        else if (destType->isIntegerTy())
        {
            if (is_u)
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI, val, destType, "cast");
            }
            else
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI, val, destType, "cast");
            }
        }
    }
    else if (fromType->isIntegerTy())
    {
        if (destType->isIntegerTy())
        {
            uint32_t fw = fromType->getIntegerBitWidth(), tw = destType->getIntegerBitWidth();
            if (fw == tw)
                return val;
            else if (fw < tw)
            {
                if (is_u)
                {
                    return ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, val, destType, "cast");
                }
                else
                {
                    return ir::builder->CreateCast(llvm::Instruction::CastOps::SExt, val, destType, "cast");
                }
            }
            else
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, val, destType, "cast");
            }
        }
        else if (destType->isFloatTy() || destType->isDoubleTy())
        {
            if (is_u)
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::UIToFP, val, destType, "cast");
            }
            else
            {
                return ir::builder->CreateCast(llvm::Instruction::CastOps::SIToFP, val, destType, "cast");
            }
        }
    }

    throw std::runtime_error("Could not coerce value to desired type");
}