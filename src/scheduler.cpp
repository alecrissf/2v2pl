#include "scheduler.hpp"

namespace sgbd
{

Lock::Resource operationResToLockRes(Operation::Resource res)
{
  switch (res)
  {
    case Operation::Resource::Area:  return Lock::Resource::Area;
    case Operation::Resource::Table: return Lock::Resource::Table;
    case Operation::Resource::Page:  return Lock::Resource::Page;
    case Operation::Resource::Row:   return Lock::Resource::Row;
  }
  return Lock::Resource::Row;
}

void Scheduler::schedule(Operation op)
{
  auto res = operationResToLockRes(op.res);
  auto shouldSchedule =
    std::visit(
      [&, tr = op.tr](auto& op) { return schedule(tr, op, res); }, op.type);

  if (shouldSchedule) m_operations.push_back(op);
  else op.tr->waiting.push_back(op);
}

bool Scheduler::schedule(Transaction *tr, Operation::Read &read, Lock::Resource res)
{
  if (tr->aborted)
    return false;

  auto lockType = Lock::readLock(read.isUpdate, false);
  bool wait = false;
  switch (res)
  {
    case Lock::Resource::Row:
      wait &= !requestRowLocks(lockType, tr, read.table);
      if (tr->aborted)
        break;
      lockType = Lock::readLock(read.isUpdate, true);

    case Lock::Resource::Page:
      wait &= !requestPageLocks(lockType, tr, read.table);
      if (tr->aborted)
        break;
      lockType = Lock::readLock(read.isUpdate, true);

    case Lock::Resource::Table:
      wait &= !requestTableLock(lockType, tr, read.table);
      if (tr->aborted)
        break;
      lockType = Lock::readLock(read.isUpdate, true);

    case Lock::Resource::Area:
      wait &= !requestAreaLock(lockType, tr, read.table);
      break;
  }
  return !wait;
}

bool Scheduler::schedule(Transaction *tr, Operation::Write &write, Lock::Resource res)
{
  if (tr->aborted)
    return false;

  auto lockType = Lock::writeLock(false);
  bool wait = false;
  switch (res)
  {
    case Lock::Resource::Row:
      wait &= !requestRowLocks(lockType, tr, write.table);
      if (tr->aborted)
        break;
      lockType = Lock::writeLock(true);

    case Lock::Resource::Page:
      wait &= !requestPageLocks(lockType, tr, write.table);
      if (tr->aborted)
        break;
      lockType = Lock::writeLock(true);

    case Lock::Resource::Table:
      wait &= !requestTableLock(lockType, tr, write.table);
      if (tr->aborted)
        break;
      lockType = Lock::writeLock(true);

    case Lock::Resource::Area:
      wait &= !requestAreaLock(lockType, tr, write.table);
      break;
  }
  return !wait;
}

bool Scheduler::schedule(Transaction *tr, Operation::Commit &commit, Lock::Resource res)
{
  if (tr->aborted)
    return false;

  for (auto it = m_lockInfo.rbegin(); it != m_lockInfo.rend(); ++it)
    if (it->tr->id == tr->id && it->status == Lock::Waiting)
      return false;

  int convertedCount = 0, totalCount = 0;
  for (auto& lock : m_lockInfo)
  {
    if (lock.tr->id == tr->id && (lock.type == Lock::Write || lock.type == Lock::IWrite))
    {
      totalCount++;
      auto rit = std::find_if(m_lockInfo.begin(), m_lockInfo.end(), [&lock](Lock& l)
      {
        return
          (l.type == Lock::Read || l.type == Lock::IRead) &&
          l.tr->id != lock.tr->id &&
          lock.table == l.table;
      });
      if (rit != m_lockInfo.end())
      {
        lock.status = Lock::Waiting;
      }
      else
      {
        lock.status = Lock::Granted;
        convertedCount++;
      }
      lock.type = lock.type == Lock::Write ? Lock::Certify : Lock::ICertify;
    }
  }

  if (convertedCount != totalCount)
    return false;

  m_lockInfo.remove_if([tr](Lock& lock)
  {
    return lock.tr->id == tr->id && (lock.type == Lock::Read || lock.type == Lock::IRead);
  });

  auto waiting = m_graph.remove(tr->id);

  for (auto id : waiting)
  {
    if (!m_graph.waitsForAny(id))
    {
      for (auto& lock : m_lockInfo)
      {
        if (lock.tr->id == id && lock.status == Lock::Waiting)
        {
          lock.status = Lock::Granted;
          for (auto& op : lock.tr->waiting)
            m_operations.push_back(op);
          lock.tr->waiting.clear();
        }
      }
    }
  }

  return true;
}

bool Scheduler::requestRowLocks(Lock::Type type, Transaction *tr, Table *t)
{
  auto conflict = getConflictLock(type, tr, t);
  auto status = conflict ? Lock::Waiting : Lock::Granted;
  for (auto& page : t->pages)
    for (auto& row : page.rows)
      m_lockInfo.emplace_back(tr, t, row.id, type, status, Lock::Resource::Row);

  if (conflict) addWaitForEdge(tr, conflict->tr);

  return !conflict;
}

bool Scheduler::requestPageLocks(Lock::Type type, Transaction *tr, Table *t)
{
  auto conflict = getConflictLock(type, tr, t);
  auto status = conflict ? Lock::Waiting : Lock::Granted;
  for (auto& page : t->pages)
    m_lockInfo.emplace_back(tr, t, npos, type, status, Lock::Resource::Page);

  if (conflict) addWaitForEdge(tr, conflict->tr);

  return !conflict;
}

bool Scheduler::requestTableLock(Lock::Type type, Transaction *tr, Table *t)
{
  auto conflict = getConflictLock(type, tr, t);
  auto status = conflict ? Lock::Waiting : Lock::Granted;
  m_lockInfo.emplace_back(tr, t, npos, type, status, Lock::Resource::Table);

  if (conflict) addWaitForEdge(tr, conflict->tr);

  return !conflict;
}

bool Scheduler::requestAreaLock(Lock::Type type, Transaction *tr, Table *t)
{
  auto conflict = getConflictLock(type, tr, t);
  auto status = conflict ? Lock::Waiting : Lock::Granted;
  m_lockInfo.emplace_back(tr, t, npos, type, status, Lock::Resource::Area);

  if (conflict) addWaitForEdge(tr, conflict->tr);

  return !conflict;
}

Lock *Scheduler::getConflictLock(Lock::Type type, Transaction *tr, Table *t)
{
  auto it = std::find_if(m_lockInfo.begin(), m_lockInfo.end(), [&](Lock& l)
  {
    if (l.tr->id == tr->id && l.status == Lock::Waiting)
      return false;

    return
      l.status != Lock::Waiting &&
      l.tr->id != tr->id &&
      l.table == t &&
      !Lock::isCompatible(type, l.type);
  });
  return it != m_lockInfo.end() ? &*it : nullptr;
}

void Scheduler::addWaitForEdge(Transaction *ti, Transaction *tj)
{
  if (ti->id == tj->id)
    return;

  if (!m_graph.add(ti->id, tj->id))
    abortTransaction(ti->timestamp < tj->timestamp ? tj : ti);
}

void Scheduler::abortTransaction(Transaction *tr)
{
  tr->aborted = true;

  m_lockInfo.remove_if([tr](Lock& l) { return l.tr->id == tr->id; });
}

} // namespace sgbd
