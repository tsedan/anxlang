#pragma once

#include <map>

#include "../anx.h"
#include "../codegen/ir.h"

namespace intr {
extern std::map<std::string, ir::Symbol> intrinsics;
ir::Symbol handle(std::string name, anx::Pos pos);
} // namespace intr
