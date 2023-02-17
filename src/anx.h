#pragma once

#include <fstream>
#include <optional>

#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace anx
{
    enum Types
    {
        ty_void,

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

    extern std::ifstream anxf; // The anx input file
    extern bool verbose;       // Whether the compiler should print what it's doing

    void perr(std::string msg);
    Types toType(std::string type);
}
