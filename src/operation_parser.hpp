#pragma once

#include "common.hpp"
#include "parser.hpp"
#include "table.hpp"
#include "transaction.hpp"

namespace sgbd
{

/// @brief Auxilia na geração de operações a partir de texto.
class OperationParser : private Parser
{
 public:
  OperationParser(std::string_view src, ResourceManager& rm, TransactionManager& tm);

  auto nextOperation() -> std::optional<Operation>;
  bool hasNext();

 private:
  ResourceManager& m_resManager;
  TransactionManager& m_trManager;
};

} // namespace sgbd
