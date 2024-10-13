#include "data/bar.hpp"
#include <ranges>

midas::Bar midas::operator>>(const std::string &line, midas::Bar &bar) {
    auto splitColumns = std::ranges::split_view(line, std::string(","));
    std::vector<std::string> columns;
    for (auto column : splitColumns) {
      columns.emplace_back(std::string_view(column));
    }
    bar =
        midas::Bar(std::stoi(columns[8]), std::stoi(columns[6]), std::stod(columns[1]),
            std::stod(columns[4]), std::stod(columns[2]), std::stod(columns[3]),
            std::stod(columns[7]), std::stod(columns[5]),
            boost::posix_time::from_iso_extended_string(columns[0])

        );
    return bar;
  }