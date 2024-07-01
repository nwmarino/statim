// Copyright (c) 2024 Nick Marino (github.com/nwmarino)

#include <string>
#include <iostream>

#include "./lex/lexer.h"

using std::string;

int main() {
  string path("../test/function/main.statim");
  Lexer lex(path);

  tstream tokens = lex.tokenize();

  std::cout << tokens.debugstr() << '\n';
}
