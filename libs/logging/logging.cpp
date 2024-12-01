#include "logging/logging.hpp"
#include <boost/core/null_deleter.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#include <boost/date_time.hpp>
#include <boost/smart_ptr.hpp>
#include <fstream>

namespace expr = boost::log::expressions;
namespace src = boost::log::sources;

void logging::initialize_logging() {
  boost::shared_ptr<boost::log::core> core = boost::log::core::get();

  typedef boost::log::sinks::synchronous_sink<
      boost::log::sinks::text_ostream_backend>
      sink_t;
  boost::shared_ptr<sink_t> sink(new sink_t());
  sink->locked_backend()->add_stream(
      boost::make_shared<std::ofstream>("midas.log"));
  sink->locked_backend()->auto_flush(true);
  sink->set_formatter(
      expr::stream
      << "[File: " << a_file << ":" << a_line << "] "
      << "[TimeStamp: "
      << expr::attr<boost::log::attributes::local_clock::value_type>(
             "TimeStamp")
      << "] "
      << "[Severity: " << expr::attr<logging::SeverityLevel>("Severity") << "] "
      << "[Channel: " << expr::attr<std::string>("Channel") << "] "
      << expr::smessage

  );

  core->add_sink(sink);
  core->add_global_attribute("TimeStamp",
                             boost::log::attributes::local_clock());
}

logging::thread_safe_logger_t
logging::create_channel_logger(const std::string &channel) {
  auto logger =
      src::severity_channel_logger_mt<logging::SeverityLevel, std::string>();
  logger.channel(channel);
  return logger;
}