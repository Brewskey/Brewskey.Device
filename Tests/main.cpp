#include "CppUnitLite/TestHarness.h"

int main()
{
  TestResult tr;
  TestRegistry::runAllTests(tr);


  return tr.getFailurecount() == 0 ? 0 : -1;
}
