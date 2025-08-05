#include "common.h"
#include "lexer.h"
#include "metadata.h"
#include "token.h"

i32 main(i32 argc, char** argv) {
    char* path = "main.sk";
    char* src = "main :: () { ret 42; }";

    SkInputFile file;
    if (skInitInputFile(path, src, &file) != SK_SUCCESS)
        return 1;

    SkLexer lexer;
    if (skInitLexer(file, &lexer) != SK_SUCCESS)
        return 1;

    SkToken token = skLexToken(lexer);
    while (token.kind != SK_TOKEN_KIND_END_OF_FILE) {
        printf("token kind: %d, token value: %s\n", token.kind, token.pValue);
        free(token.pValue);
        token = skLexToken(lexer);
    }

    skDestroyInputFile(&file);
    skDestroyLexer(&lexer);
    
    return 0;
}
