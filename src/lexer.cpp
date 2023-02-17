#include "anx.h"
#include "lexer.h"

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file.
//===---------------------------------------------------------------------===//

lex::Token lex::tok;

// Get the next token from the input file and update the global token variable
void lex::eat()
{
    static char lch = ' ';

    while (isspace(lch))
        lch = anx::anxf.get();

    tok.val = lch;

    if (lch == EOF)
    {
        tok.tok = tok_eof;
        return;
    }

    if (isalpha(lch))
    {
        while (isalnum(lch = anx::anxf.get()))
            tok.val += lch;

        if (tok.val == "fn")
            tok.tok = tok_fn;
        else if (tok.val == "pub")
            tok.tok = tok_pub;
        else if (tok.val == "ret")
            tok.tok = tok_ret;
        else if (tok.val == "var")
            tok.tok = tok_var;
        else if (tok.val == "if")
            tok.tok = tok_if;
        else if (tok.val == "else")
            tok.tok = tok_else;
        else
            tok.tok = tok_identifier;

        return;
    }

    if (isdigit(lch))
    {
        while (isdigit(lch = anx::anxf.get()) || lch == '.')
            tok.val += lch;

        tok.tok = tok_number;
        return;
    }

    if (lch == '#')
    {
        do
            lch = anx::anxf.get();
        while (lch != EOF && lch != '\n' && lch != '\r');

        if (lch == EOF)
            tok.tok = tok_eof;
        else
            eat();

        return;
    }

    char old = lch;
    lch = anx::anxf.get();

    switch (old)
    {
    case ';':
        tok.tok = tok_eol;
        return;
    case ',':
        tok.tok = tok_comma;
        return;
    case '{':
        tok.tok = tok_curlys;
        return;
    case '}':
        tok.tok = tok_curlye;
        return;
    case '(':
        tok.tok = tok_parens;
        return;
    case ')':
        tok.tok = tok_parene;
        return;
    case ':':
        tok.tok = tok_type;
        return;
    case '*':
    case '/':
    case '+':
    case '-':
    case '%':
        tok.tok = tok_binop;
        return;
    case '<':
    case '>':
        tok.tok = tok_binop;
        if (lch == '=')
        {
            tok.val += lch;
            lch = anx::anxf.get();
        }
        return;
    case '=':
        if (lch == '=')
        {
            tok.val += lch;
            lch = anx::anxf.get();
            tok.tok = tok_binop;
        }
        else
            tok.tok = tok_assign;
        return;
    case '!':
        if (lch == '=')
        {
            tok.val += lch;
            lch = anx::anxf.get();
            tok.tok = tok_binop;
        }
        else
            tok.tok = tok_unop;
        return;
    }

    anx::perr("Invalid token: '" + std::string(1, old) + "'");
}

void lex::exp(lex::TokEnum token, std::string msg)
{
    if (tok.tok != token)
        anx::perr(msg);
}
