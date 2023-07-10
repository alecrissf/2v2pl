#pragma once

#include "common.hpp"

#include <unordered_set>
#include <unordered_map>

namespace sgbd
{

/// @brief Grafo de espera de transações.
class WaitForGraph
{
 public:
  /// @brief Adiciona uma aresta no grafo de espera onde ti -> tj.
  /// @param ti ID da transação i.
  /// @param tj ID da transação j.
  /// @return true se foi possível adicionar e false se houve ciclo.
  bool add(usize ti, usize tj);

  /// @brief Remove uma transação do grafo.
  /// @param tr ID da transação.
  /// @return conjunto de transações que esperavam pela removida.
  auto remove(usize tr) -> std::unordered_set<usize>;

  /// @brief Verifica se uma transação i espera por j.
  /// @param ti ID da transação i.
  /// @param tj ID da transação j.
  /// @return true se i depende de j.
  bool waitsFor(usize ti, usize tj);

  /// @brief Verifica se a transação espera por alguma outra.
  /// @param tr ID da transação
  /// @return true se tr esperar por outra transação qualquer.
  bool waitsForAny(usize tr);

  const std::unordered_map<usize, std::unordered_set<usize>>& getNodes() const
  {
    return m_nodes;
  }

 private:
  /// @brief Verifica se existe um ciclo entre start e end.
  /// @param start
  /// @param end
  /// @return true se start -> ... -> end.
  bool hasCycle(usize start, usize end);

 private:
  std::unordered_map<usize, std::unordered_set<usize>> m_nodes;
};

} // namespace sgbd
