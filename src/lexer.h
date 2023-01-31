#pragma once

#include <string>
#include <vector>

enum TokEnum
{
    // general
    tok_eof, // end of file
    tok_eos, // end of statement

    // grouping
    tok_comma,  // ,
    tok_curlys, // curly start
    tok_curlye, // curly end
    tok_parens, // paren start
    tok_parene, // paren end

    // functions
    tok_fn,   // function
    tok_ret,  // return
    tok_void, // void

    // variables
    tok_var,    // variable
    tok_type,   // :
    tok_assign, // =

    // comparison
    tok_equal, // ==

    // operators
    tok_mul, // *
    tok_div, // /
    tok_add, // +
    tok_sub, // -

    // control
    tok_if, // if

    // identifiers
    tok_identifier, // identifier
    tok_integer,    // literal integer
    tok_boolean     // literal boolean
};

struct Token
{
    TokEnum tok;
    std::string val;
    union
    {
        int32_t i32val;
        bool bval;
    };
};

extern std::vector<Token> tokens;

void gen_tokens();
