#pragma once

#include "llvm/IR/BasicBlock.h"

#include "anx.h"

namespace ty {
enum Type {
  ty_void,
  ty_bool,

  ty_i8,
  ty_i16,
  ty_i32,
  ty_i64,
  ty_i128,

  ty_u8,
  ty_u16,
  ty_u32,
  ty_u64,
  ty_u128,

  ty_f32,
  ty_f64,
};

bool isSInt(Type ty);
bool isUInt(Type ty);
bool isSingle(Type ty);
bool isDouble(Type ty);
bool isVoid(Type ty);
bool isBool(Type ty);
uint32_t width(Type ty);

std::string toString(Type type);
Type fromString(std::string type, bool allow_void, anx::Pos pos = {},
                size_t s = 0);
llvm::Type *toLLVM(Type ty, bool allow_void, anx::Pos pos = {}, size_t s = 0);
} // namespace ty
