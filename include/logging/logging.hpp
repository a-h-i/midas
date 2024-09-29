#pragma once

#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
namespace logging {

enum SeverityLevel { debug, info, warning, error, critical };
template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream,
           logging::SeverityLevel severity) {
  switch (severity) {
  case logging::SeverityLevel::critical:
    stream << "critical";
    break;
  case logging::SeverityLevel::error:
    stream << "error";
    break;
    case logging::SeverityLevel::warning:
    stream << "warning";
    break;
  case logging::SeverityLevel::info:
    stream << "info";
    break;
  case logging::SeverityLevel::debug:
    stream << "debug";
    break;
  }
  return stream;
}
void initialize_logging();

using thread_safe_logger_t = boost::log::sources::severity_channel_logger_mt<SeverityLevel, std::string> ;

thread_safe_logger_t create_channel_logger(const std::string &channel);

} // namespace logging

BOOST_LOG_ATTRIBUTE_KEYWORD(a_file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(a_line, "Line", std::string)

#define LOG(lg, sev)                                                           \
  BOOST_LOG_SEV(lg, sev) << boost::log::add_value("File", __FILE__)            \
                         << boost::log::add_value("Line",                      \
                                                  std::to_string(__LINE__))

#define CRITICAL_LOG(lg) LOG(lg, logging::SeverityLevel::critical)
#define ERROR_LOG(lg) LOG(lg, logging::SeverityLevel::error)
#define WARNING_LOG(lg) LOG(lg, logging::SeverityLevel::warning)
#define INFO_LOG(lg) LOG(lg, logging::SeverityLevel::info)
#define DEBUG_LOG(lg) LOG(lg, logging::SeverityLevel::debug)
