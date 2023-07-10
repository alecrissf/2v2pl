#include "transaction.hpp"

namespace sgbd
{

usize TransactionManager::s_currentTimestamp = 0;

Transaction *TransactionManager::registerTransaction(usize id)
{
  return &m_transactions.try_emplace(id, id, s_currentTimestamp++).first->second;
}

Transaction *TransactionManager::get(usize id)
{
  return m_transactions.contains(id) ? &m_transactions[id] : nullptr;
}

} // namespace sgbd
