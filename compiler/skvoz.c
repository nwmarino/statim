#include "common.h"
#include "lexer.h"
#include "logger.h"
#include "metadata.h"
#include "token.h"

inline SkString skStringFromChar(const char c) {
    SkString string;
    string.size = 1;
    string.pData = (char*) calloc(sizeof(char), 2);
    strncpy(string.pData, &c, 1);
    string.pData[1] = '\0';

    return string;
}

SkString skStringFromCStr(const char *pStr) {
    SkString string;

    string.size = strlen(pStr);
    string.pData = (char*) calloc(sizeof(char), string.size + 1);
    strncpy(string.pData, pStr, string.size);
    string.pData[string.size + 1] = '\0';

    return string;
}

i32 main(i32 argc, char** argv) {
    SkString src = skStringFromCStr("main :: () { ret 42; }");

    SkInputFile file;
    if (skInitInputFile(src, src, &file) != SK_SUCCESS)
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
    free(src.pData);
    
    return 0;
}
