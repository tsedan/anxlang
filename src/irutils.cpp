#include <map>

#include "anx.h"
#include "ir.h"
#include "ast.h"

ir::Symbol ir::search(std::string name)
{
    std::map<std::string, ir::Symbol>::iterator sym;

    for (auto it = ir::symbols.rbegin(); it != ir::symbols.rend(); ++it)
        if ((sym = it->find(name)) != it->end())
            return sym->second;

    throw std::runtime_error("Unidentified symbol '" + name + "'");
}

llvm::Type *ir::get_type(std::string ty, bool allow_void)
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