/**
 * main.cpp
 *
 * Program entry point.
 */


#include "core/pch.hpp"  // IWYU pragma: export
#include "utils/utils.hpp"


int main()
{
  #ifndef WSC_NOCATCH
  try 
  {
  #endif

  WSC_ASSERT(false,  "Testing assert.");

  #ifndef WSC_NOCATCH
  } catch (const std::system_error& sysE) {
    std::cerr << "FATAL EXCEPTION:\n" << sysE.what() << std::flush;
  }
  #endif

}