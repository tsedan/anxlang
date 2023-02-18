#include <map>

#include "anx.h"
#include "ir.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// Utils - This module defines utility functions.
//===---------------------------------------------------------------------===//

anx::Symbol anx::Symbol::coerce(Types toType)
{
    Types fromType = ty();
    llvm::Value *v = val();
    llvm::Type *destType = getType(toType, true);

    if (isVoid(fromType))
    {
        if (isVoid(toType))
            return *this;
    }
    else if (isSingle(fromType))
    {
        if (isSingle(toType))
            return *this;
        else if (isDouble(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPExt, v, destType, "cast"), toType);
        else if (isSInt(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI, v, destType, "cast"), toType);
        else if (isUInt(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI, v, destType, "cast"), toType);
        else if (isBool(toType))
        {
            float cmp = 0;
            return Symbol(ir::builder->CreateFCmpONE(v, llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(cmp)), "cmp"), toType);
        }
    }
    else if (isDouble(fromType))
    {
        if (isSingle(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPTrunc, v, destType, "cast"), toType);
        else if (isDouble(toType))
            return *this;
        else if (isSInt(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI, v, destType, "cast"), toType);
        else if (isUInt(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI, v, destType, "cast"), toType);
        else if (isBool(toType))
        {
            double cmp = 0;
            return Symbol(ir::builder->CreateFCmpONE(v, llvm::ConstantFP::get(*ir::ctx, llvm::APFloat(cmp)), "cmp"), toType);
        }
    }
    else if (isSInt(fromType))
    {
        if (isSingle(toType) || isDouble(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::SIToFP, v, destType, "cast"), toType);
        else if (isSInt(toType))
        {
            uint32_t fw = width(fromType), tw = width(toType);
            if (fw == tw)
                return *this;
            if (fw < tw)
                return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::SExt, v, destType, "cast"), toType);
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
        else if (isUInt(toType))
        {
            uint32_t fw = width(fromType), tw = width(toType);
            if (fw == tw)
                return Symbol(v, toType);
            if (fw < tw)
                return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, v, destType, "cast"), toType);
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
        else if (isBool(toType))
            return Symbol(ir::builder->CreateICmpNE(v, llvm::ConstantInt::get(*ir::ctx, llvm::APInt(width(fromType), 0)), "cmp"), toType);
    }
    else if (isUInt(fromType) || isBool(fromType))
    {
        if (isSingle(toType) || isDouble(toType))
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::UIToFP, v, destType, "cast"), toType);
        else if (isSInt(toType))
        {
            uint32_t fw = width(fromType), tw = width(toType);
            if (fw == tw)
                return Symbol(v, toType);
            if (fw < tw)
                return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, v, destType, "cast"), toType);
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
        else if (isUInt(toType))
        {
            uint32_t fw = width(fromType), tw = width(toType);
            if (fw == tw)
                return *this;
            if (fw < tw)
                return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, v, destType, "cast"), toType);
            return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
        else if (isBool(toType))
        {
            if (isBool(fromType))
                return *this;

            return Symbol(ir::builder->CreateICmpNE(v, llvm::ConstantInt::get(*ir::ctx, llvm::APInt(width(fromType), 0)), "cmp"), toType);
        }
    }

    std::cerr << fromType << " " << toType << '\n';

    perr("Could not coerce to the desired type");
}

bool anx::isSingle(Types ty)
{
    return ty == ty_f32;
}

bool anx::isDouble(Types ty)
{
    return ty == ty_f64;
}

bool anx::isSInt(Types ty)
{
    return ty == ty_i8 || ty == ty_i16 || ty == ty_i32 || ty == ty_i64 || ty == ty_i128;
}

bool anx::isUInt(Types ty)
{
    return ty == ty_u8 || ty == ty_u16 || ty == ty_u32 || ty == ty_u64 || ty == ty_u128;
}

bool anx::isVoid(Types ty)
{
    return ty == ty_void;
}

bool anx::isBool(Types ty)
{
    return ty == ty_bool;
}

uint32_t anx::width(Types ty)
{
    switch (ty)
    {
    case ty_void:
        return 0;
    case ty_bool:
        return 1;
    case ty_f32:
        return 32;
    case ty_f64:
        return 64;
    case ty_i8:
    case ty_u8:
        return 8;
    case ty_i16:
    case ty_u16:
        return 16;
    case ty_i32:
    case ty_u32:
        return 32;
    case ty_i64:
    case ty_u64:
        return 64;
    case ty_i128:
    case ty_u128:
        return 128;
    }
}

anx::Types anx::toType(std::string type)
{
    if (type == "void")
        return ty_void;

    if (type == "bool")
        return ty_bool;

    if (type == "i8")
        return ty_i8;
    if (type == "i16")
        return ty_i16;
    if (type == "i32")
        return ty_i32;
    if (type == "i64")
        return ty_i64;
    if (type == "i128")
        return ty_i128;

    if (type == "u8")
        return ty_u8;
    if (type == "u16")
        return ty_u16;
    if (type == "u32")
        return ty_u32;
    if (type == "u64")
        return ty_u64;
    if (type == "u128")
        return ty_u128;

    if (type == "f32")
        return ty_f32;
    if (type == "f64")
        return ty_f64;

    perr("Invalid type '" + type + "'");
}

llvm::Type *anx::getType(Types ty, bool allow_void)
{
    switch (ty)
    {
    case ty_f32:
        return llvm::Type::getFloatTy(*ir::ctx);
    case ty_f64:
        return llvm::Type::getDoubleTy(*ir::ctx);
    case ty_bool:
        return llvm::Type::getInt1Ty(*ir::ctx);
    case ty_i8:
    case ty_u8:
        return llvm::Type::getInt8Ty(*ir::ctx);
    case ty_i16:
    case ty_u16:
        return llvm::Type::getInt16Ty(*ir::ctx);
    case ty_i32:
    case ty_u32:
        return llvm::Type::getInt32Ty(*ir::ctx);
    case ty_i64:
    case ty_u64:
        return llvm::Type::getInt64Ty(*ir::ctx);
    case ty_i128:
    case ty_u128:
        return llvm::Type::getInt128Ty(*ir::ctx);
    case ty_void:
        if (allow_void)
            return llvm::Type::getVoidTy(*ir::ctx);
        else
            perr("Void type not allowed here");
    default:
        perr("Unrecognized type");
    }
}
