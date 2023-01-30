#include "anx.h"
#include "lexer.h"

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file
//
// See lexer.h for the Token enum declaration.
//===---------------------------------------------------------------------===//

static Token token;

// Get the next token from the input file
void fill_token()
{
    static char lch = ' ';

    while (isspace(lch))
        lch = anxf.get();

    if (lch == EOF)
    {
        token.tok = tok_eof;
        return;
    }

    token.val = lch;

    if (isalpha(lch))
    {
        while (isalnum(lch = anxf.get()))
            token.val += lch;

        if (token.val == "fn")
            token.tok = tok_fn;
        else if (token.val == "ret")
            token.tok = tok_ret;
        else if (token.val == "var")
            token.tok = tok_var;
        else if (token.val == "if")
            token.tok = tok_if;
        else if (token.val == "void")
            token.tok = tok_void;
        else if (token.val == "true")
        {
            token.bval = true;
            token.tok = tok_boolean;
        }
        else if (token.val == "false")
        {
            token.bval = false;
            token.tok = tok_boolean;
        }
        else
            token.tok = tok_identifier;

        return;
    }

    if (isdigit(lch))
    {
        while (isdigit(lch = anxf.get()))
            token.val += lch;

        token.i32val = atoi(token.val.c_str());
        token.tok = tok_integer;
        return;
    }

    if (lch == '#')
    {
        do
            lch = anxf.get();
        while (lch != EOF && lch != '\n' && lch != '\r');

        if (lch == EOF)
            token.tok = tok_eof;
        else
            fill_token();

        return;
    }

    char old = lch;
    lch = anxf.get();

    switch (old)
    {
    case ';':
        token.tok = tok_eos;
        return;
    case ',':
        token.tok = tok_comma;
        return;
    case '{':
        token.tok = tok_curlys;
        return;
    case '}':
        token.tok = tok_curlye;
        return;
    case '(':
        token.tok = tok_parens;
        return;
    case ')':
        token.tok = tok_parene;
        return;
    case ':':
        token.tok = tok_type;
        return;
    case '*':
        token.tok = tok_mul;
        return;
    case '/':
        token.tok = tok_div;
        return;
    case '+':
        token.tok = tok_add;
        return;
    case '-':
        token.tok = tok_sub;
        return;
    case '=':
        if (lch == '=')
        {
            token.val += lch;
            token.tok = tok_equal;
        }
        else
            token.tok = tok_assign;
        return;
    }

    perr("Invalid token: '" + std::string(1, old) + "'");
}

std::vector<Token> gen_tokens()
{
    std::vector<Token> ret;

    fill_token();
    while (token.tok != tok_eof)
    {
        ret.push_back(token);
        fill_token();
    }

    return ret;
}
