#include "lock.hpp"

namespace sgbd
{

bool Lock::isCompatible(Type li, Type lj)
{
  switch (li)
  {
    case Read:
      switch (lj)
      {
        case Read:
        case Write:
        case Update:
        case IRead:
        case IWrite:
        case IUpdate:
          return true;
      }
      break;
    case Write:
      switch (lj)
      {
        case Read:
        case IRead:
          return true;
      }
      break;
    case IRead:
      switch (lj)
      {
        case Read:
        case Write:
        case Update:
        case IRead:
        case IWrite:
        case IUpdate:
          return true;
      }
      break;
    case IWrite:
      switch (lj)
      {
        case Read:
        case IRead:
        case IWrite:
        case IUpdate:
          return true;
      }
      break;
    case IUpdate:
      switch (lj)
      {
        case IRead:
        case IWrite:
        case IUpdate:
          return true;
      }
      break;
  }
  return false;
}

Lock::Type Lock::readLock(bool isUpdate, bool isIntent)
{
  return isUpdate ? (isIntent ? IUpdate : Update) : (isIntent ? IRead : Read);
}

Lock::Type Lock::writeLock(bool isIntent)
{
  return isIntent ? IWrite : Write;
}

Lock::Type Lock::certifyLock(bool isIntent)
{
  return isIntent ? ICertify : Certify;
}

} // namespace sgbd

/*

|       | rl_i  | wl_i  | ul_i  | cl_i  | irl_i | iwl_i | iul_i | icl_i |
| rl_j  |   +   |   +   |   -   |   -   |   +   |   +   |   -   |   -   |
| wl_j  |   +   |   -   |   -   |   -   |   +   |   -   |   -   |   -   |
| ul_j  |   +   |   -   |   -   |   -   |   +   |   -   |   -   |   -   |
| cl_j  |   -   |   -   |   -   |   -   |   -   |   -   |   -   |   -   |
| irl_j |   +   |   +   |   -   |   -   |   +   |   +   |   +   |   -   |
| iwl_j |   +   |   -   |   -   |   -   |   +   |   +   |   +   |   -   |
| iul_j |   +   |   -   |   -   |   -   |   +   |   +   |   +   |   -   |
| icl_j |   -   |   -   |   -   |   -   |   -   |   -   |   -   |   -   |

*/
