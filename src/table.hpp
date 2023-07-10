#pragma once

#include "common.hpp"

#include <unordered_map>
#include <vector>
#include <string>

namespace sgbd
{

/// @brief Modelo de tabela de banco de dados.
struct Table
{
  struct Area
  {
    std::string id;
  };

  struct Row
  {
    usize id;
  };

  struct Page
  {
    std::vector<Row> rows;
  };

  std::string name;
  Area* area;
  std::vector<Page> pages;
};

/// @brief Gerencia as tabelas do banco de dados.
class ResourceManager
{
 public:
  /// @brief Cria uma área se ja não existir.
  /// @param name Nome da área.
  void createArea(const std::string& name);

  /// @brief Cria uma tabela se já não existir.
  /// @param name Nome da tabela.
  /// @param area Nome da área (cria a área se não existir).
  void createTable(const std::string& name, const std::string& area);

  /// @brief Insere uma tupla na tabela em uma página específica.
  /// @param table Nome da tabela.
  /// @param row Tupla.
  /// @param page Página.
  void insertRow(const std::string& table, const Table::Row& row, uint page = 0);

  /// @brief Busca um ponteiro para a tabela se existir.
  /// @param name Nome da tabela.
  /// @return Ponteiro para a tabela ou nullptr se não existir.
  Table* getTable(const std::string& name);

 private:
  std::unordered_map<std::string, Table::Area> m_areas;
  std::unordered_map<std::string, Table> m_tables;
};

} // namespace sgbd
