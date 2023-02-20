#include "anx.h"
#include "lexer.h"

#include <iostream>

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file.
//===---------------------------------------------------------------------===//

lex::Token lex::tok;              // current token being parsed
size_t lex::cr, lex::cc;          // current row and column
size_t lex::lr, lex::lc, lex::ls; // last row, column, and size

size_t tr = 0, tc = 0; // true current row and column

char grab()
{
    if (tc == anx::file[tr].size())
    {
        if (tr == anx::file.size() - 1)
            return EOF;

        tc = 0, tr++;

        return '\n';
    }

    return anx::file[tr][tc++];
}

// Get the next token from the input file and update the global token variable
void lex::eat()
{
    static char lch = ' ';

    while (isspace(lch))
        lch = grab();

    lr = cr, lc = cc, ls = tok.val.size();
    cr = tr, cc = tc - 1;

    tok.val = lch;

    if (lch == EOF)
    {
        tok.tok = tok_eof;
        return;
    }

    if (isalpha(lch))
    {
        while (isalnum(lch = grab()))
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
        while (isdigit(lch = grab()) || lch == '.')
            tok.val += lch;

        tok.tok = tok_number;
        return;
    }

    if (lch == '#')
    {
        do
            lch = grab();
        while (lch != EOF && lch != '\n' && lch != '\r');

        if (lch == EOF)
            tok.tok = tok_eof;
        else
            eat();

        return;
    }

    char old = lch;
    lch = grab();

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
            lch = grab();
        }
        return;
    case '=':
        if (lch == '=')
        {
            tok.val += lch;
            lch = grab();
            tok.tok = tok_binop;
        }
        else
            tok.tok = tok_assign;
        return;
    case '!':
        if (lch == '=')
        {
            tok.val += lch;
            lch = grab();
            tok.tok = tok_binop;
        }
        else
            tok.tok = tok_unop;
        return;
    }

    anx::perr("invalid token found", cr, cc);
}

void lex::exp(lex::TokEnum token, std::string msg)
{
    if (tok.tok != token)
        anx::perr(msg, lr, lc + ls);
}
