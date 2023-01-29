#include "tokenizer.h"

//===---------------------------------------------------------------------===//
// Tokenizer - This converts an input file into tokens
//===---------------------------------------------------------------------===//

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
