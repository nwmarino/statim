#include "include/lexer.h"

i32 main(i32 argc, char** argv) {
    char* path = "main.sk";
    char* src = "main :: () { ret 42; }";

    StmInputFile file;
    file.pPath = path;
    file.pContents = src;

    StmLexer lexer;
    if (stmInitLexer(&file, &lexer) != STM_SUCCESS)
        return 1;

    const StmToken* token = stmLexToken(lexer);
    while (token->kind != STM_TOKEN_KIND_END_OF_FILE) {
        printf("token kind: %d, token value: %s\n", token->kind, token->pValue);
        token = stmLexToken(lexer);
    }

    stmDestroyLexer(&lexer);
    
    return EXIT_SUCCESS;
}
