#pragma once

#include "common.hpp"
#include "lock.hpp"
#include "transaction.hpp"
#include "wait_for_graph.hpp"

#include <vector>
#include <list>

namespace sgbd
{

/// @brief Escalonador 2v2pl
class Scheduler
{
 public:
  const std::vector<Operation>& getScheduling() const { return m_operations; }
  const std::list<Lock>& getLockInfo() const { return m_lockInfo; }
  const WaitForGraph& getWaitForGraph() const { return m_graph; }

  /// @brief Escalona uma operação ou coloca em espera.
  /// @param op
  void schedule(Operation op);

 private:
  /// @brief Gerencia os bloqueios do novo escalonamento.
  /// @param tr Ponteiro para a transação.
  /// @param read
  /// @param res Nível de granulosidade.
  /// @return true se for possível escalonar
  bool schedule(Transaction* tr, Operation::Read& read, Lock::Resource res);

  /// @brief Gerencia os bloqueios do novo escalonamento.
  /// @param tr Ponteiro para a transação.
  /// @param write
  /// @param res Nível de granulosidade.
  /// @return true se for possível escalonar
  bool schedule(Transaction* tr, Operation::Write& write, Lock::Resource res);

  /// @brief Gerencia os bloqueios do novo escalonamento.
  /// @param tr Ponteiro para a transação.
  /// @param commit
  /// @param res Nível de granulosidade.
  /// @return true se for possível escalonar
  bool schedule(Transaction* tr, Operation::Commit& commit, Lock::Resource res);

  bool requestRowLocks(Lock::Type type, Transaction* tr, Table* t);
  bool requestPageLocks(Lock::Type type, Transaction* tr, Table* t);
  bool requestTableLock(Lock::Type type, Transaction* tr, Table* t);
  bool requestAreaLock(Lock::Type type, Transaction* tr, Table* t);

  /// @brief Procura por um bloqueio conflitante.
  /// @param type
  /// @param tr
  /// @param t
  /// @return Ponteiro para o primeiro bloqueio conflitante e nullptr se não encontrar.
  Lock* getConflictLock(Lock::Type type, Transaction* tr, Table* t);

  /// @brief Adiciona uma aresta no grafo de espera e lida com aborts.
  /// @param ti
  /// @param tj
  void addWaitForEdge(Transaction* ti, Transaction* tj);

  /// @brief Aborta a transação.
  /// @param tr
  void abortTransaction(Transaction* tr);

 private:
  std::vector<Operation> m_operations;
  std::list<Lock> m_lockInfo;
  WaitForGraph m_graph;
};

} // namespace sgbd
