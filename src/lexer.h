#pragma once

#include <string>
#include <vector>

namespace lex
{
    enum TokEnum
    {
        // general
        tok_eof, // end of file
        tok_eol, // end of line

        // grouping
        tok_comma,  // ,
        tok_curlys, // curly start
        tok_curlye, // curly end
        tok_parens, // paren start
        tok_parene, // paren end

        // functions
        tok_fn,  // function
        tok_pub, // public decorator
        tok_ret, // return

        // variables
        tok_var,    // variable
        tok_type,   // :
        tok_assign, // =

        // comparison / arithmetic
        tok_binop, // binary operation
        tok_unop,  // unary operation

        // control
        tok_if,   // if
        tok_else, // else

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

    extern Token tok;
    void eat();
    void exp(TokEnum token, std::string msg);

    int prio(const std::string &op);
}
