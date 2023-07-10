#pragma once

#include "common.hpp"
#include "lock.hpp"
#include "transaction.hpp"

#include <string_view>
#include <optional>
#include <vector>

namespace sgbd
{

/// @brief Classe para análise de operações.
class Parser
{
 public:
  enum class TokenType
  {
    Identifier,
    Number,
    Read,
    Write,
    Commit,
    RowL,
    TabL,
    PagL,
    AreL,
    UpdL,
    LeftParen,
    RightParen,
    Eof,
    Error,
  };

  struct Token
  {
    TokenType type;
    std::string_view lexeme;
  };

 public:
  Parser(std::string_view src);

  auto readToken() -> Token;
  bool isAtEnd();

  auto consume(TokenType type) -> std::optional<Token>;
  auto consume() -> Token;
  auto consumeNumber() -> std::optional<int>;

  auto peekToken(usize at = 0) -> Token;
  bool match(TokenType type);
  auto last() -> Token;

 private:
  auto makeToken(TokenType type) -> Token;
  auto makeError(std::string_view msg) -> Token;

  char advance();
  bool match(char expected);
  char peek(usize at = 0);
  void skipWhitespace();

  auto identifierType(std::string_view lexeme) -> TokenType;

 private:
  std::string_view m_src;
  usize m_start;
  usize m_current;

  std::vector<Token> m_tokens;
  usize m_readTokens;
};

} // namespace sgbd
