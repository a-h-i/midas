
#include <boost/program_options.hpp>
#include <iostream>

#include "midas/version.h"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  po::options_description appOptionsDesc("Midas AlgoTrading options");
  appOptionsDesc.add_options()("help,h", "prints this message")(
      "version,v", "prints program version");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, appOptionsDesc), vm);
  po::notify(vm);
  if (vm.empty() || vm.count("help")) {
    std::cout << appOptionsDesc << '\n';
  }

  if (vm.count("version")) {
    std::cout << "Midas version " << MIDAS_VERSION << '\n';
  }

  return 0;
}