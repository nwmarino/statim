// Copyright (c) 2024 Nick Marino (github.com/nwmarino)

#ifndef TSTREAM_H
#define TSTREAM_H

#include <vector>

#include "token.h"

using std::vector;
using std::size_t;

class tstream {
  public:
    tstream(vector<Token> __tokens);
    Token next();
    size_t size();

  private:
    vector<Token> __tokens;
    int __currit;
};

#endif  // TSTREAM_H