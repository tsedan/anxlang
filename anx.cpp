#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

//===---------------------------------------------------------------------===//
// Tokenizer
//===---------------------------------------------------------------------===//

enum Token
{
    tok_eof = -1,
    tok_eol = -2,

    tok_scopes = -3,
    tok_scopee = -4,

    tok_fn = -5,
    tok_ret = -6,

    tok_var = -7,
    tok_let = -8,

    tok_type = -9,

    tok_if = -10,
    tok_else = -11,
    tok_while = -12,

    tok_identifier = -13,
    tok_number = -14,
};

static std::string IdentifierStr;
static double NumVal;
