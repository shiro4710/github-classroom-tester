#include <iostream>
using std::string;

#include "terminal/terminal.hpp"
#include "tester/tester.hpp"

int main(int argc, char *argv[]) {
  // Main method
  std::map<string, string> options = my_terminal::ParseOptions(argc, argv);
  my_tester::RunTests(options);
}
