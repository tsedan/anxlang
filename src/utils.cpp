#include <map>

#include "anx.h"
#include "ir.h"
#include "ast.h"

anx::Symbol anx::Symbol::coerce(Types toType)
{
    Types typ = ty();
    llvm::Value *v = val();
    llvm::Type *destType = getType(toType, true);

    if (isVoid(typ))
    {
        if (isVoid(toType))
            return *this;
    }
    else if (isSFl(typ))
    {
        if (isSFl(toType))
            return *this;
        else if (isDFl(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPExt, v, destType, "cast"), toType);
        else if (isSInt(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI, v, destType, "cast"), toType);
        else if (isUInt(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI, v, destType, "cast"), toType);
    }
    else if (isDFl(typ))
    {
        if (isSFl(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPTrunc, v, destType, "cast"), toType);
        else if (isDFl(toType))
            return *this;
        else if (isSInt(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI, v, destType, "cast"), toType);
        else if (isUInt(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI, v, destType, "cast"), toType);
    }
    else if (isSInt(typ))
    {
        if (isSFl(toType) || isDFl(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::SIToFP, v, destType, "cast"), toType);
        else if (isSInt(toType))
        {
            uint32_t fw = width(typ), tw = width(toType);
            if (fw == tw)
                return *this;
            if (fw < tw)
                return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::SExt, v, destType, "cast"), toType);
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
        else if (isUInt(toType))
        {
            uint32_t fw = width(typ), tw = width(toType);
            if (fw == tw)
                return anx::Symbol(v, toType);
            if (fw < tw)
                return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, v, destType, "cast"), toType);
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
    }
    else if (isUInt(typ))
    {
        if (isSFl(toType) || isDFl(toType))
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::UIToFP, v, destType, "cast"), toType);
        else if (isSInt(toType))
        {
            uint32_t fw = width(typ), tw = width(toType);
            if (fw == tw)
                return anx::Symbol(v, toType);
            if (fw < tw)
                return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, v, destType, "cast"), toType);
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
        else if (isUInt(toType))
        {
            uint32_t fw = width(typ), tw = width(toType);
            if (fw == tw)
                return *this;
            if (fw < tw)
                return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt, v, destType, "cast"), toType);
            return anx::Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc, v, destType, "cast"), toType);
        }
    }

    throw std::runtime_error("Could not coerce value to the desired type");
}

bool anx::isSFl(Types ty)
{
    return ty == ty_f32;
}

bool anx::isDFl(Types ty)
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

uint32_t anx::width(Types ty)
{
    switch (ty)
    {
    case ty_void:
        return 0;
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

    throw std::runtime_error("Invalid type '" + type + "'");
}

llvm::Type *anx::getType(Types ty, bool allow_void)
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
