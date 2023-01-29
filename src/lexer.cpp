#include "anx.h"
#include "lexer.h"

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file
//===---------------------------------------------------------------------===//

Token token;

std::string idstr;
int32_t i32val;
bool bval;

// Get the next token from the input file
Token get_token()
{
    static char lch = ' ';

    while (isspace(lch))
        lch = anxf.get();

    if (isalpha(lch))
    {
        idstr = lch;
        while (isalnum(lch = anxf.get()))
            idstr += lch;

        if (idstr == "fn")
            return tok_fn;
        if (idstr == "ret")
            return tok_ret;
        if (idstr == "var")
            return tok_var;
        if (idstr == "if")
            return tok_if;
        if (idstr == "void")
            return tok_void;

        if (idstr == "true")
        {
            bval = true;
            return tok_boolean;
        }
        if (idstr == "false")
        {
            bval = false;
            return tok_boolean;
        }

        return tok_identifier;
    }

    if (isdigit(lch))
    {
        std::string nstr;
        do
        {
            nstr += lch;
            lch = anxf.get();
        } while (isdigit(lch));

        i32val = atoi(nstr.c_str());
        return tok_integer;
    }

    if (lch == '#')
    {
        do
            lch = anxf.get();
        while (lch != EOF && lch != '\n' && lch != '\r');

        if (lch != EOF)
            return get_token();
    }

    if (lch == ';')
    {
        lch = anxf.get();
        return tok_eos;
    }

    if (lch == ',')
    {
        lch = anxf.get();
        return tok_comma;
    }

    if (lch == '{')
    {
        lch = anxf.get();
        return tok_curlys;
    }

    if (lch == '}')
    {
        lch = anxf.get();
        return tok_curlye;
    }

    if (lch == '(')
    {
        lch = anxf.get();
        return tok_parens;
    }

    if (lch == ')')
    {
        lch = anxf.get();
        return tok_parene;
    }

    if (lch == ':')
    {
        lch = anxf.get();
        return tok_type;
    }

    if (lch == '*')
    {
        lch = anxf.get();
        return tok_mul;
    }

    if (lch == '/')
    {
        lch = anxf.get();
        return tok_div;
    }

    if (lch == '+')
    {
        lch = anxf.get();
        return tok_add;
    }

    if (lch == '-')
    {
        lch = anxf.get();
        return tok_sub;
    }

    if (lch == '=')
    {
        lch = anxf.get();

        if (lch == '=')
        {
            lch = anxf.get();
            return tok_equal;
        }

        return tok_assign;
    }

    if (lch == EOF)
        return tok_eof;

    printf("Unknown token '%c'", lch);
    exit(1);
}

// Get the next token from the input file, setting the global token variable
Token next_token()
{
    return token = get_token();
}
