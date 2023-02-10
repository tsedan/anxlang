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

extern std::ifstream anxf; // The anx input file
extern bool verbose;       // Whether the compiler should print what it's doing

void perr(std::string msg);
