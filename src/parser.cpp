#include "parser.hpp"

#include <charconv>

namespace sgbd
{

bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

bool isAlpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

Parser::Parser(std::string_view src)
  : m_src(src),
    m_start(0),
    m_current(0),
    m_readTokens(0) {}

auto Parser::readToken() -> Token
{
  skipWhitespace();
  m_start = m_current;

  if (isAtEnd())
    return makeToken(TokenType::Eof);

  char c = advance();

  if (isDigit(c))
  {
    while (!isAtEnd() && isDigit(peek()))
      advance();

    return makeToken(TokenType::Number);
  }

  if (isAlpha(c))
  {
    while (!isAtEnd() && isAlpha(peek()))
      advance();

    return makeToken(identifierType(m_src.substr(m_start, m_current - m_start)));
  }

  switch (c)
  {
    case '(': return makeToken(TokenType::LeftParen);
    case ')': return makeToken(TokenType::RightParen);
  }

  return makeError("Unexpected character.");
}

bool Parser::isAtEnd()
{
  return m_current >= m_src.length();
}

auto Parser::consume(TokenType type) -> std::optional<Token>
{
  return match(type) ? last() : std::optional<Token>();
}

auto Parser::consume() -> Token
{
  peekToken();
  return m_tokens[m_readTokens++];
}

auto Parser::consumeNumber() -> std::optional<int>
{
  if (auto token = consume(TokenType::Number))
  {
    auto lexeme = token->lexeme;
    int n = 0;
    if (std::from_chars(lexeme.begin(), lexeme.end(), n).ec == std::errc())
      return n;
  }
  return std::optional<int>();
}

auto Parser::peekToken(usize at) -> Token
{
  while (m_tokens.size() <= m_readTokens + at)
    m_tokens.push_back(readToken());

  return m_tokens[m_readTokens + at];
}

bool Parser::match(TokenType type)
{
  if (peekToken().type != type)
    return false;
  consume();
  return true;
}

auto Parser::last() -> Token
{
  return m_tokens[m_readTokens - 1];
}

auto Parser::makeToken(TokenType type) -> Token
{
  return { type, m_src.substr(m_start, m_current - m_start) };
}

auto Parser::makeError(std::string_view msg) -> Token
{
  return { TokenType::Error, msg };
}

char Parser::advance()
{
  if (m_current >= m_src.length())
    return '\0';
  return m_src[m_current++];
}

bool Parser::match(char expected)
{
  if (isAtEnd() || m_src[m_current] != expected)
    return false;
  advance();
  return true;
}

char Parser::peek(usize at)
{
  if (m_current + at >= m_src.length())
    return '\0';
  return m_src[m_current + at];
}

void Parser::skipWhitespace()
{
  while (!isAtEnd())
  {
    switch (peek())
    {
      case ' ':
      case '\r':
      case '\t':
      case '\n':
        advance();
        break;
      default:
        return;
    }
  }
}

auto Parser::identifierType(std::string_view lexeme) -> TokenType
{
  if (lexeme.length() == 1)
  {
    switch (lexeme[0])
    {
      case 'r': return TokenType::Read;
      case 'w': return TokenType::Write;
      case 'c': return TokenType::Commit;
    }
  }
  else
  {
    if (lexeme == "rowl") return TokenType::RowL;
    if (lexeme == "tabl") return TokenType::TabL;
    if (lexeme == "pagl") return TokenType::PagL;
    if (lexeme == "arel") return TokenType::AreL;
    if (lexeme == "updl") return TokenType::UpdL;
  }

  return TokenType::Identifier;
}

} // namespace sgbd
