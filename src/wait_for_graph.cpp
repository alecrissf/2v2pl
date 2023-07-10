#include "wait_for_graph.hpp"

namespace sgbd
{

bool WaitForGraph::add(usize ti, usize tj)
{
  if (hasCycle(tj, ti))
    return false;

  m_nodes[ti].emplace(tj);
  m_nodes.try_emplace(tj);
  return true;
}

auto WaitForGraph::remove(usize tr) -> std::unordered_set<usize>
{
  std::unordered_set<usize> waiting;
  m_nodes.erase(tr);
  for (auto& node : m_nodes)
    if (node.second.erase(tr))
      waiting.insert(node.first);

  return waiting;
}

bool WaitForGraph::waitsFor(usize ti, usize tj)
{
  return m_nodes.contains(ti) && m_nodes[ti].contains(tj);
}

bool WaitForGraph::waitsForAny(usize tr)
{
  return m_nodes.contains(tr) && !m_nodes[tr].empty();
}

bool WaitForGraph::hasCycle(usize start, usize end)
{
  if (start == end || waitsFor(start, end))
    return true;

  if (m_nodes.contains(start) && m_nodes[start].empty())
    return false;

  for (auto& tr : m_nodes[start])
    if (hasCycle(tr, end))
      return true;

  return false;
}

} // namespace sgbd
