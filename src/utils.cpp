#include <map>

#include "codegen/ir.h"
#include "frontend/ast.h"
#include "utils.h"

//===---------------------------------------------------------------------===//
// Utils - This module defines miscellaneous utility functions.
//===---------------------------------------------------------------------===//

ir::Symbol ir::Symbol::coerce(ty::Type toType, anx::Pos pos, size_t s) {
  ty::Type fromType = typ();
  llvm::Value *v = val();
  llvm::Type *destType = toLLVM(toType, true);

  if (isVoid(fromType)) {
    if (isVoid(toType))
      return *this;
  } else if (isSingle(fromType)) {
    if (isSingle(toType))
      return *this;
    else if (isDouble(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPExt,
                                            v, destType, "cast"),
                    toType);
    else if (isSInt(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI,
                                            v, destType, "cast"),
                    toType);
    else if (isUInt(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI,
                                            v, destType, "cast"),
                    toType);
    else if (isBool(toType))
      return Symbol(
          ir::builder->CreateFCmpUNE(
              v, llvm::ConstantFP::get(*ir::ctx, llvm::APFloat((float)0)),
              "cmp"),
          toType);
  } else if (isDouble(fromType)) {
    if (isSingle(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPTrunc,
                                            v, destType, "cast"),
                    toType);
    else if (isDouble(toType))
      return *this;
    else if (isSInt(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToSI,
                                            v, destType, "cast"),
                    toType);
    else if (isUInt(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::FPToUI,
                                            v, destType, "cast"),
                    toType);
    else if (isBool(toType))
      return Symbol(
          ir::builder->CreateFCmpUNE(
              v, llvm::ConstantFP::get(*ir::ctx, llvm::APFloat((double)0)),
              "cmp"),
          toType);
  } else if (isSInt(fromType)) {
    if (isSingle(toType) || isDouble(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::SIToFP,
                                            v, destType, "cast"),
                    toType);
    else if (isSInt(toType)) {
      uint32_t fw = width(fromType), tw = width(toType);
      if (fw == tw)
        return *this;
      if (fw < tw)
        return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::SExt,
                                              v, destType, "cast"),
                      toType);
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc,
                                            v, destType, "cast"),
                    toType);
    } else if (isUInt(toType)) {
      uint32_t fw = width(fromType), tw = width(toType);
      if (fw == tw)
        return Symbol(v, toType);
      if (fw < tw)
        return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt,
                                              v, destType, "cast"),
                      toType);
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc,
                                            v, destType, "cast"),
                    toType);
    } else if (isBool(toType))
      return Symbol(
          ir::builder->CreateICmpNE(
              v,
              llvm::ConstantInt::get(*ir::ctx, llvm::APInt(width(fromType), 0)),
              "cmp"),
          toType);
  } else if (isUInt(fromType) || isBool(fromType)) {
    if (isSingle(toType) || isDouble(toType))
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::UIToFP,
                                            v, destType, "cast"),
                    toType);
    else if (isSInt(toType)) {
      uint32_t fw = width(fromType), tw = width(toType);
      if (fw == tw)
        return Symbol(v, toType);
      if (fw < tw)
        return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt,
                                              v, destType, "cast"),
                      toType);
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc,
                                            v, destType, "cast"),
                    toType);
    } else if (isUInt(toType)) {
      uint32_t fw = width(fromType), tw = width(toType);
      if (fw == tw)
        return *this;
      if (fw < tw)
        return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::ZExt,
                                              v, destType, "cast"),
                      toType);
      return Symbol(ir::builder->CreateCast(llvm::Instruction::CastOps::Trunc,
                                            v, destType, "cast"),
                    toType);
    } else if (isBool(toType)) {
      if (isBool(fromType))
        return *this;

      return Symbol(
          ir::builder->CreateICmpNE(
              v,
              llvm::ConstantInt::get(*ir::ctx, llvm::APInt(width(fromType), 0)),
              "cmp"),
          toType);
    }
  }

  anx::perr("cannot coerce type '" + toString(fromType) + "' to '" +
                toString(toType) + "'",
            pos, s);
}

bool ty::isSingle(Type ty) { return ty == ty_f32; }

bool ty::isDouble(Type ty) { return ty == ty_f64; }

bool ty::isSInt(Type ty) {
  return ty == ty_i8 || ty == ty_i16 || ty == ty_i32 || ty == ty_i64 ||
         ty == ty_i128;
}

bool ty::isUInt(Type ty) {
  return ty == ty_u8 || ty == ty_u16 || ty == ty_u32 || ty == ty_u64 ||
         ty == ty_u128;
}

bool ty::isVoid(Type ty) { return ty == ty_void; }

bool ty::isBool(Type ty) { return ty == ty_bool; }

uint32_t ty::width(Type ty) {
  switch (ty) {
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

ty::Type ty::fromString(std::string type, bool allow_void, anx::Pos pos,
                        size_t s) {
  if (type == "void") {
    if (allow_void)
      return ty_void;
    anx::perr("void is only valid as a function return type", pos, s);
  }

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

  anx::perr("unrecognized type", pos, s);
}

std::string ty::toString(Type type) {
  switch (type) {
  case ty_void:
    return "void";
  case ty_bool:
    return "bool";
  case ty_f32:
    return "f32";
  case ty_f64:
    return "f64";
  case ty_i8:
    return "i8";
  case ty_u8:
    return "u8";
  case ty_i16:
    return "i16";
  case ty_u16:
    return "u16";
  case ty_i32:
    return "i32";
  case ty_u32:
    return "u32";
  case ty_i64:
    return "i64";
  case ty_u64:
    return "u64";
  case ty_i128:
    return "i128";
  case ty_u128:
    return "u128";
  }
}

llvm::Type *ty::toLLVM(Type ty, bool allow_void, anx::Pos pos, size_t s) {
  switch (ty) {
  case ty_bool:
    return llvm::Type::getInt1Ty(*ir::ctx);
  case ty_f32:
    return llvm::Type::getFloatTy(*ir::ctx);
  case ty_f64:
    return llvm::Type::getDoubleTy(*ir::ctx);
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
      anx::perr("void is only valid as a function return type", pos, s);
  }
}
