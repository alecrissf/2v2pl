#include "scheduler.hpp"
#include "parser.hpp"
#include "table.hpp"
#include "transaction.hpp"
#include "operation_parser.hpp"

#include <iostream>
#include <iomanip>
#include <string>

void populateData(sgbd::ResourceManager& resManager, int pagec, int rowc)
{
  resManager.createTable("x", "A");
  resManager.createTable("y", "A");
  resManager.createTable("z", "A");
  resManager.createTable("u", "B");
  resManager.createTable("v", "B");
  for (auto tab : { "x", "y", "z", "u", "v" })
    for (int id = 0, p = 0; p < pagec; p = (id++ / rowc))
      resManager.insertRow(tab, { (sgbd::usize)id }, p);
}

void showOperation(const sgbd::Operation& op)
{
  std::cout << op.tr->id << " - ";
  switch (op.type.index())
  {
    case 0:
      std::cout << "r: " << std::get<0>(op.type).table->name;
      break;
    case 1:
      std::cout << "w: " << std::get<1>(op.type).table->name;
      break;
    case 2:
      std::cout << 'c';
      break;
  }
  std::cout << '\n';
}

std::string_view showLockType(sgbd::Lock::Type type)
{
  switch (type)
  {
    case sgbd::Lock::Read:     return "r";
    case sgbd::Lock::Write:    return "w";
    case sgbd::Lock::Update:   return "u";
    case sgbd::Lock::Certify:  return "c";
    case sgbd::Lock::IRead:    return "ir";
    case sgbd::Lock::IWrite:   return "iw";
    case sgbd::Lock::IUpdate:  return "iu";
    case sgbd::Lock::ICertify: return "ic";
  }
  return "?";
}

std::string_view showLockStatus(sgbd::Lock::Status status)
{
  switch (status)
  {
    case sgbd::Lock::Granted:    return "granted";
    case sgbd::Lock::Converting: return "converting";
    case sgbd::Lock::Waiting:    return "waiting";
  }
  return "?";
}

std::string_view showLockRes(sgbd::Lock::Resource res)
{
  switch (res)
  {
    case sgbd::Lock::Resource::Area:  return "area";
    case sgbd::Lock::Resource::Table: return "table";
    case sgbd::Lock::Resource::Page:  return "page";
    case sgbd::Lock::Resource::Row:   return "row";
  }
  return "?";
}

void showLock(const sgbd::Lock& l)
{
  std::cout << " | "
    << std::setw(4)  << l.tr->id                 << " | "
    << std::setw(10) << l.table->name            << " | "
    << std::setw(4)  << showLockType(l.type)     << " | "
    << std::setw(10) << showLockStatus(l.status) << " | "
    << std::setw(5)  << showLockRes(l.res)       << " |\n";
}

void showWaitForGraph(const sgbd::WaitForGraph& graph)
{
  for (auto& n : graph.getNodes())
  {
    if (n.second.empty())
      continue;

    std::cout << n.first << " -> ";
    for (auto& t : n.second)
      std::cout << t << " ";
    std::cout << '\n';
  }
}

int main()
{
  sgbd::ResourceManager resManager;  populateData(resManager, 2, 5);
  sgbd::TransactionManager trManager;
  sgbd::Scheduler scheduler;

  std::cout <<
    "~[ Implementação 2v2pl ]~\n"
    "Comandos:\n"
    "    exit          - sai do programa\n"
    "    show          - mostra o estado do escalonamento atual\n"
    "    locki         - mostra o estado dos bloqueios\n"
    "    test1 e test2 - executam operações de teste\n"
    "    <op><trid>(<obj> [<upd>] [<res>])\n"
    "      onde:\n"
    "        <op>:   r, w, c\n"
    "        <trid>: id da transação\n"
    "        <obj>:  nome da tabela\n"
    "        <upd>:  updl\n"
    "                - bloqueio de update (válido somente para leitura)\n"
    "        <res>:  rowl, tabl, pagl, arel\n"
    "                - granulosidade de bloqueio\n\n"
    "Tabelas disponíveis: x, y, z, u, v\n"
    "    - 2 páginas por tabela\n"
    "    - 5 tuplas por página\n";

  for (std::string line; (std::cout << "> "), std::getline(std::cin, line);)
  {
    if (line == "exit")
      break;

    if (line == "show")
    {
      for (auto& op : scheduler.getScheduling())
        showOperation(op);

      continue;
    }

    if (line == "locki")
    {
      std::cout << " | "
        << std::setw(4)  << "trid"   << " | "
        << std::setw(10) << "obj"    << " | "
        << std::setw(4)  << "lock"   << " | "
        << std::setw(10) << "status" << " | "
        << std::setw(5)  << "res"    << " |\n";
      for (auto& lock : scheduler.getLockInfo())
        showLock(lock);

      showWaitForGraph(scheduler.getWaitForGraph());

      continue;
    }

    if (line == "test1")
    {
      line = "r4(v)r3(y)r1(y)r1(x)w2(u)r2(x)w1(y)r2(y)c1w4(u)r3(x)c4w2(x)c2w3(u)w3(z)c3";
      std::cout << "testando: " << line << '\n';
    }
    else if (line == "test2")
    {
      line = "r1(y)w2(u)r2(x)r3(u)w2(y)r3(v)c2c3w1(v)c1";
      std::cout << "testando: " << line << '\n';
    }

    auto parser = sgbd::OperationParser(line, resManager, trManager);

    while (parser.hasNext())
    {
      if (auto op = parser.nextOperation())
      {
        scheduler.schedule(*op);
        if (op->tr->aborted)
          std::cout << "transação ( " << op->tr->id << " ) foi [abortada]";
      }
    }
  }

  return 0;
}
