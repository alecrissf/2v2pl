#include "table.hpp"

namespace sgbd
{

void ResourceManager::createArea(const std::string &name)
{
  m_areas.try_emplace(name, name);
}

void ResourceManager::createTable(const std::string &name, const std::string &area)
{
  createArea(area);
  m_tables.try_emplace(name, name, &m_areas[area]);
}

void ResourceManager::insertRow(const std::string &table, const Table::Row &row, uint page)
{
  if (!m_tables.contains(table))
    return;

  auto& tab = m_tables[table];

  if (page >= tab.pages.size())
    tab.pages.resize(page + 1);

  tab.pages[page].rows.emplace_back(row);
}

Table *ResourceManager::getTable(const std::string &name)
{
  if (m_tables.contains(name))
    return &m_tables[name];
  return nullptr;
}

} // namespace sgbd
