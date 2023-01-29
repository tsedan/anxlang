#include "tokenizer.h"

//===---------------------------------------------------------------------===//
// Anx main - The entry point of the Anx compiler.
//===---------------------------------------------------------------------===//

int main()
{
    Token t;

    while ((t = gettok()) != tok_eof)
        printf("%d ", t);
}
