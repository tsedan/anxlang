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
    tok_fn,  // function
    tok_ret, // return

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
};

static std::string idstr;
static int32_t i32val;

// Get the next token from stdin
static Token gettok()
{
    static int lch = ' ';

    // skip whitespace
    while (isspace(lch))
        lch = getchar();

    if (isalpha(lch))
    {
        idstr = lch;
        while (isalnum(lch = getchar()))
            idstr += lch;

        if (idstr == "fn")
            return tok_fn;
        if (idstr == "ret")
            return tok_ret;
        if (idstr == "var")
            return tok_var;
        if (idstr == "if")
            return tok_if;

        return tok_identifier;
    }

    if (isdigit(lch))
    {
        std::string nstr;
        do
        {
            nstr += lch;
            lch = getchar();
        } while (isdigit(lch));

        i32val = atoi(nstr.c_str());
        return tok_integer;
    }

    if (lch == '#')
    {
        do
            lch = getchar();
        while (lch != EOF && lch != '\n' && lch != '\r');

        if (lch != EOF)
            return gettok();
    }

    if (lch == ';')
    {
        lch = getchar();
        return tok_eos;
    }

    if (lch == ',')
    {
        lch = getchar();
        return tok_comma;
    }

    if (lch == '{')
    {
        lch = getchar();
        return tok_curlys;
    }

    if (lch == '}')
    {
        lch = getchar();
        return tok_curlye;
    }

    if (lch == '(')
    {
        lch = getchar();
        return tok_parens;
    }

    if (lch == ')')
    {
        lch = getchar();
        return tok_parene;
    }

    if (lch == ':')
    {
        lch = getchar();
        return tok_type;
    }

    if (lch == '*')
    {
        lch = getchar();
        return tok_mul;
    }

    if (lch == '/')
    {
        lch = getchar();
        return tok_div;
    }

    if (lch == '+')
    {
        lch = getchar();
        return tok_add;
    }

    if (lch == '-')
    {
        lch = getchar();
        return tok_sub;
    }

    if (lch == '=')
    {
        lch = getchar();

        if (lch == '=')
        {
            lch = getchar();
            return tok_equal;
        }

        return tok_assign;
    }

    if (lch == EOF)
        return tok_eof;

    printf("Unknown token '%c'", lch);
    exit(1);
}
