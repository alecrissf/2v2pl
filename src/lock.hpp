#pragma once

#include "common.hpp"
#include "table.hpp"
#include "transaction.hpp"

namespace sgbd
{

/// @brief Informações de bloqueio.
struct Lock
{
  enum Type : ubyte
  {
    Read,
    Write,
    Update,
    Certify,
    IRead,
    IWrite,
    IUpdate,
    ICertify,
  };

  enum Status : ubyte
  {
    Granted,
    Converting,
    Waiting,
  };

  enum class Resource : ubyte
  {
    Area,
    Table,
    Page,
    Row,
  };

  Transaction* tr;
  Table* table;
  usize obj;
  Type type;
  Status status;
  Resource res;

  /// @brief Verifica se os bloqueios li e lj são compativeis.
  /// @param li Bloqueio i.
  /// @param lj Bloqueio j.
  /// @return true se compativeis.
  static bool isCompatible(Type li, Type lj);

  static Type readLock(bool isUpdate, bool isIntent);
  static Type writeLock(bool isIntent);
  static Type certifyLock(bool isIntent);
};

} // namespace sgbd
