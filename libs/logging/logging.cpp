#include "logging/logging.hpp"
#include <boost/core/null_deleter.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#include <boost/date_time.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>

namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

void logging::initialize_logging() {
  boost::shared_ptr<boost::log::core> core = boost::log::core::get();
  boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend =
      boost::make_shared<boost::log::sinks::text_ostream_backend>();
  backend->add_stream(
      boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
  backend->auto_flush(true);

  typedef boost::log::sinks::synchronous_sink<decltype(backend)::element_type>
      sink_t;
  boost::shared_ptr<sink_t> sink(new sink_t(backend));
  sink->set_formatter(
      expr::stream
      << "[File: " << a_file << ":" << a_line << "] "
      << "[TimeStamp: "
      << expr::attr<boost::log::attributes::local_clock::value_type>(
             "TimeStamp")
      << "] "
      << "[Severity: " << expr::attr<logging::SeverityLevel>("Severity") << "] "
      << expr::smessage);

  core->add_sink(sink);
  core->add_global_attribute("TimeStamp",
                             boost::log::attributes::local_clock());
}