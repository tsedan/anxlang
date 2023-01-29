#include "anx.h"
#include "tokenizer.h"

//===---------------------------------------------------------------------===//
// Anx main - The entry point of the Anx compiler.
//===---------------------------------------------------------------------===//

std::ifstream anxf;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: anx <filename>\n");
        return 1;
    }

    anxf.open(argv[1]);

    if (!anxf.is_open())
    {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    Token t;

    while ((t = gettok()) != tok_eof)
        printf("%d ", t);
}
