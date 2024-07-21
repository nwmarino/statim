/// Copyright 2024 Nick Marino (github.com/nwmarino)

#include <iostream>
#include <vector>

#include "../include/token.h"
#include "../include/tstream.h"

/**
 * Construct a new token stream.
 * 
 * @param __tokens A token vector to copy.
 */
tstream::tstream(std::vector<Token> __tokens)
{
  Token terminate;
  terminate.type = Terminate;
  __tokens.push_back(terminate);
  
  this->__tokens = __tokens;
  this->__currit = 0;
  this->curr = __tokens[__currit];
}


/**
 * Iterate to the next token in this buffer.
 */
void tstream::next()
{
  if (curr.type == Terminate)
    return;

  this->__currit++;
  this->curr = __tokens[__currit];
}


/**
 * Peek at the next token in this buffer.
 * 
 * @return The next token in this buffer.
 */
Token tstream::peek()
{
  if (curr.type == Terminate)
    return curr;
  return __tokens[__currit + 1];
}


/**
 * Fetch the size of this stream.
 * 
 * @return The size of this stream.
 */
std::size_t tstream::size() {
  return __tokens.size();
}


/**
 * Print the contents of this stream by type.
 */
void tstream::print() {
  std::string result;
  for (int i = 0; i < this->__tokens.size(); i++) {
    switch (this->__tokens[i].type) {
      case SetParen: std::cout << "SET_PARENTHESES\n"; break;
      case EndParen: std::cout << "END_PARENTHESES\n"; break;
      case SetBrace: std::cout << "SET_BLOCK\n"; break;
      case EndBrace: std::cout << "END_BLOCK\n"; break;
      case Semicolon: std::cout << "SEMICOLON\n"; break;
      case Separator: std::cout << "SEPARATOR\n"; break;
      case Comma: std::cout << "COMMA\n"; break;
      case Arrow: std::cout << "ARROW\n"; break;
      case AssignOperator: std::cout << "ASSIGN_OP\n"; break;
      case AddOperator: std::cout << "ADD_OP\n"; break;
      case SubOperator: std::cout << "SUB_OP\n"; break;
      case MultOperator: std::cout << "MULT_OP\n"; break;
      case DivOperator: std::cout << "DIV_OP\n"; break;
      case PowerOperator: std::cout << "POWER_OP\n"; break;
      case BoolKeyword: std::cout << "BOOL_KEY\n"; break;
      case IntKeyword: std::cout << "INT_KEY\n"; break;
      case StringKeyword: std::cout << "STR_KEY\n"; break;
      case FloatKeyword: std::cout << "FLOAT_KEY\n"; break;
      case CharKeyword: std::cout << "CHAR_KEY\n"; break;
      case LetKeyword: std::cout << "LET_KEY\n"; break;
      case FixKeyword: std::cout << "FIX_KEY\n"; break;
      case FunctionKeyword: std::cout << "FUNCTION_KEY\n"; break;
      case ReturnKeyword: std::cout << "RETURN_KEY\n"; break;
      case Comment: std::cout << "COMMENT\n"; break;
      case Identifier: std::cout << "IDENTIFIER " << this->__tokens[i].value << '\n'; break;
      case Boolean: std::cout << "BOOLEAN " << this->__tokens[i].value << '\n'; break;
      case Integer: std::cout << "INTEGER " << this->__tokens[i].value << '\n';  break;
      case Float: std::cout << "FLOAT " << this->__tokens[i].value << '\n';  break;
      case String: std::cout << "STRING " << this->__tokens[i].value << '\n';  break;
      case Char: std::cout << "CHAR " << this->__tokens[i].value << '\n';  break;
      case IfKeyword: std::cout << "IF\n"; break;
      case ElseKeyword: std::cout << "ELSE\n"; break;
      case Terminate:
        break;
    }
  }
}