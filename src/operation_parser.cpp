#include "operation_parser.hpp"

#include <iostream>

namespace sgbd
{

OperationParser::OperationParser(std::string_view src, ResourceManager &rm,
  TransactionManager &tm)
    : Parser(src), m_resManager(rm), m_trManager(tm) {}

auto OperationParser::nextOperation() -> std::optional<Operation>
{
  auto op = consume().type;
  switch (op)
  {
    case Parser::TokenType::Read:
    case Parser::TokenType::Write:
    case Parser::TokenType::Commit:
      break;
    case Parser::TokenType::Error:
      std::cerr << "Error: " << last().lexeme << '\n';
    default:
      return {};
  }

  auto trid = consumeNumber();
  if (!trid)
    return {};

  auto opType = std::optional<Operation::Type>();
  auto res = Operation::Resource::Row;

  if (op != Parser::TokenType::Commit)
  {
    if (!consume(Parser::TokenType::LeftParen))
      return {};

    auto tidTok = consume(Parser::TokenType::Identifier);
    if (!tidTok)
      return {};

    auto table = m_resManager.getTable(std::string(tidTok->lexeme));
    if (!table)
      return {};

    switch (op)
    {
      case Parser::TokenType::Read:
        opType = Operation::Read { table, match(Parser::TokenType::UpdL) };
        break;
      case Parser::TokenType::Write:
        opType = Operation::Write { table };
        break;
    }

    if      (match(TokenType::RowL)) res = Operation::Resource::Row;
    else if (match(TokenType::TabL)) res = Operation::Resource::Table;
    else if (match(TokenType::PagL)) res = Operation::Resource::Page;
    else if (match(TokenType::AreL)) res = Operation::Resource::Area;

    if (!consume(Parser::TokenType::RightParen))
      return {};
  }
  else opType = Operation::Commit{};

  if (!opType)
    return {};

  return Operation { m_trManager.registerTransaction(*trid), *opType, res };
}

bool OperationParser::hasNext()
{
  return !isAtEnd();
}

} // namespace sgbd
