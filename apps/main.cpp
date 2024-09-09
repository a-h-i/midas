
#include <boost/program_options.hpp>
#include <iostream>

#include "logging/logging.hpp"
#include "midas/version.h"
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
namespace po = boost::program_options;




int main(int argc, char *argv[]) {
  logging::initialize_logging();
  boost::log::sources::severity_channel_logger<logging::SeverityLevel, std::string> lg(boost::log::keywords::channel =
                                                              "cmdline", boost::log::keywords::severity = logging::SeverityLevel::info);
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