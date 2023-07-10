#pragma once

#include "common.hpp"
#include "table.hpp"

#include <unordered_map>
#include <variant>
#include <list>

namespace sgbd
{

struct Operation;

/// @brief Informações de uma transação.
struct Transaction
{
  usize id;
  usize timestamp;
  bool aborted = false;
  std::list<Operation> waiting;
};

/// @brief Operação de uma transação.
struct Operation
{
  struct Read
  {
    Table* table;
    bool isUpdate = false;
  };

  struct Write
  {
    Table* table;
  };

  struct Commit {};

  using Type = std::variant<Read, Write, Commit>;

  enum TypeIndex : ubyte
  {
    ReadI,
    WriteI,
    CommitI,
  };

  enum class Resource : ubyte
  {
    Area,
    Table,
    Page,
    Row,
  };

  Transaction* tr;
  Type type;
  Resource res;
};

/// @brief Gerenciador de transações.
class TransactionManager
{
 public:
  /// @brief Registra uma nova transação se não existir.
  /// @param id
  Transaction* registerTransaction(usize id);

  /// @brief Retorna um ponteiro para uma transação.
  /// @param id
  /// @return Ponteiro para a transação ou nullptr se não existir.
  Transaction* get(usize id);

 private:
  std::unordered_map<usize, Transaction> m_transactions;

 private:
  static usize s_currentTimestamp;
};

} // namespace sgbd
