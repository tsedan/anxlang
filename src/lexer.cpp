#include "anx.h"
#include "lexer.h"

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file
//
// See lexer.h for the Token enum declaration.
//===---------------------------------------------------------------------===//

std::vector<Token> tokens;

static Token token;

// Get the next token from the input file
void fill_token()
{
    static char lch = ' ';

    while (isspace(lch))
        lch = anxf.get();

    token.val = lch;

    if (lch == EOF)
    {
        token.tok = tok_eof;
        return;
    }

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
        else if (token.val == "else")
            token.tok = tok_else;
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
    case '/':
    case '+':
    case '-':
        token.tok = tok_binop;
        return;
    case '<':
    case '>':
        token.tok = tok_binop;
        if (lch == '=')
        {
            token.val += lch;
            lch = anxf.get();
        }
        return;
    case '=':
        if (lch == '=')
        {
            token.val += lch;
            lch = anxf.get();
            token.tok = tok_binop;
        }
        else
            token.tok = tok_assign;
        return;
    case '!':
        if (lch == '=')
        {
            token.val += lch;
            lch = anxf.get();
            token.tok = tok_binop;
        }
        else
            token.tok = tok_unop;
        return;
    }

    perr("Invalid token: '" + std::string(1, old) + "'");
}

void gen_tokens()
{
    tokens.clear();

    do
    {
        fill_token();
        tokens.push_back(token);
    } while (token.tok != tok_eof);
}

int get_priority(const std::string &op)
{
    if (op == "+" || op == "-")
        return 0;
    else if (op == "*" || op == "/")
        return 1;
    else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        return 2;

    return -1;
}
