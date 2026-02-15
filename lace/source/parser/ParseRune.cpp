//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"

using namespace lace;

void Parser::parse_rune_decorators(std::vector<Rune*>& runes) {
    if (!expect(Token::Sign))
        return;

    static std::unordered_map<std::string, Rune::Type> table = {
        { "intrinsic", Rune::Intrinsic },
        { "public", Rune::Public },
        { "private", Rune::Private },
    };

    if (expect(Token::OpenBrack)) 
    {
        // '[' means this is a delimited list of runes, so parse runes until a
        // ']' is found.
        
        while (!expect(Token::CloseBrack)) 
        {
            if (!match(Token::Identifier))
                log::fatal("expected identifier", log::Span(m_file, loc()));

            auto it = table.find(curr().value);
            if (it == table.end())
                log::error("unknown rune: " + curr().value);

            next();
            runes.push_back(new Rune(it->second, {}));

            if (expect(Token::CloseBrack))
                break;

            if (!expect(Token::Comma))
                log::fatal("expected ','", log::Span(m_file, loc()));

        }
    } 
    else 
    {
        // No '[' means this is a single rune.

        if (!match(Token::Identifier))
            log::fatal("expected identifier", log::Span(m_file, loc()));

        auto it = table.find(curr().value);
        if (it == table.end())
            log::error("unknown rune: " + curr().value);

        next();
        runes.push_back(new Rune(it->second, {}));
    }
}
