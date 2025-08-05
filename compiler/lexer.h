#ifndef SKVOZ_LEXER_H_
#define SKVOZ_LEXER_H_

#include "metadata.h"
#include "token.h"

typedef struct {
    SkInputFile     file;
    char*           pBuffer;
    SkMetadata      meta;
    u32             position;
}* SkLexer;

SkResult skInitLexer(SkInputFile file, SkLexer* pLexer);
SkResult skDestroyLexer(SkLexer* pLexer);
SkToken skLexToken(SkLexer lexer);

#endif // SKVOZ_LEXER_H_
