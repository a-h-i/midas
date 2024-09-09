#pragma once

#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
namespace logging {

enum SeverityLevel { debug, info, warning, error, critical };
template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream,
           logging::SeverityLevel &severity) {
  switch (severity) {
  case logging::SeverityLevel::critical:
    stream << "critical";
    break;
  case logging::SeverityLevel::debug:
    stream << "debug";
    break;
  case logging::SeverityLevel::error:
    stream << "error";
    break;
  case logging::SeverityLevel::info:
    stream << "info";
    break;
  case logging::SeverityLevel::warning:
    stream << "warning";
    break;
  }
  return stream;
}
void initialize_logging();

} // namespace logging

BOOST_LOG_ATTRIBUTE_KEYWORD(a_file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(a_line, "Line", std::string)

#define LOG(lg, sev)                                                           \
  BOOST_LOG_SEV(lg, sev) << boost::log::add_value("File", __FILE__)            \
                         << boost::log::add_value("Line",                      \
                                                  std::to_string(__LINE__))
