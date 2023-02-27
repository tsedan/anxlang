#include "../anx.h"
#include "lexer.h"

#include <iostream>

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file.
//===---------------------------------------------------------------------===//

std::vector<std::string> lex::file;
lex::Token lex::tok;              // current token being parsed
size_t lex::cr, lex::cc;          // current row and column
size_t lex::lr, lex::lc, lex::ls; // last row, column, and size

size_t tr = 0, tc = 0; // true current row and column

char grab()
{
    if (tc == lex::file[tr].size())
    {
        if (tr == lex::file.size() - 1)
            return EOF;

        tc = 0, tr++;

        return '\n';
    }

    return lex::file[tr][tc++];
}

void lex::read(std::string filename)
{
    std::ifstream stream(filename);
    if (!stream.is_open())
        anx::perr("could not open file '" + filename + "'");

    for (std::string line; std::getline(stream, line);)
        file.push_back(line);
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
        else if (tok.val == "while")
            tok.tok = tok_while;
        else
            tok.tok = tok_identifier;

        return;
    }

    if (isdigit(lch))
    {
        if (lch == '0')
        {
            lch = grab();

            if (lch == 'x')
            {
                do
                {
                    tok.val += lch;
                    lch = grab();
                } while (isxdigit(lch) || lch == '_');

                if (lch == '.')
                    anx::perr("hexadecimal float literal is not supported", cr, cc, tok.val.size());

                if (lch == 'i' || lch == 'u')
                {
                    do
                        tok.val += lch;
                    while (isdigit(lch = grab()));
                }

                tok.tok = tok_number;
                return;
            }
            else if (lch == 'b')
            {
                do
                {
                    tok.val += lch;
                    lch = grab();
                } while (lch == '0' || lch == '1' || lch == '_');

                if (isdigit(lch))
                    anx::perr("invalid digit in binary literal", tr, tc - 1);

                if (lch == '.' || lch == 'f')
                    anx::perr("binary float literal is not supported", cr, cc, tok.val.size());

                if (lch == 'i' || lch == 'u')
                {
                    do
                        tok.val += lch;
                    while (isdigit(lch = grab()));
                }

                tok.tok = tok_number;
                return;
            }
            else if (lch == 'o')
            {
                do
                {
                    tok.val += lch;
                    lch = grab();
                } while ((lch >= '0' && lch <= '7') || lch == '_');

                if (isdigit(lch))
                    anx::perr("invalid digit in octal literal", tr, tc - 1);

                if (lch == '.' || lch == 'f')
                    anx::perr("octal float literal is not supported", cr, cc, tok.val.size());

                if (lch == 'i' || lch == 'u')
                {
                    do
                        tok.val += lch;
                    while (isdigit(lch = grab()));
                }

                tok.tok = tok_number;
                return;
            }
            else if (isdigit(lch) || lch == '.' || lch == '_')
            {
                tok.val += lch;
            }
            else
                goto numtype;
        }

        while (isdigit(lch = grab()) || lch == '.' || lch == '_')
            tok.val += lch;

    numtype:
        if (lch == 'i' || lch == 'u' || lch == 'f')
        {
            do
                tok.val += lch;
            while (isdigit(lch = grab()));
        }

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
        tok.tok = tok_colon;
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
    case '\'':
        size_t st = cc;

        switch (lch)
        {
        case '\'':
            anx::perr("cannot have empty character literal", tr, cc, 2);
        case '\\':
            lch = grab();
            switch (lch)
            {
            case '0':
                tok.val = '\0';
                break;
            case 'n':
                tok.val = '\n';
                break;
            case '\'':
                tok.val = '\'';
                break;
            default:
                anx::perr("unrecognized escape character", tr, tc - 1);
            }
            break;
        default:
            tok.val = lch;
        }

        lch = grab();

        if (lch != '\'')
            anx::perr("missing apostrophe or literal too large for a single character", cr, st, tc - st);
        lch = grab();

        tok.tok = tok_character;
        return;
    }

    anx::perr("invalid token found", cr, cc);
}

void lex::exp(lex::TokEnum token, std::string msg)
{
    if (tok.tok != token)
        anx::perr(msg, lr, lc + ls);
}
