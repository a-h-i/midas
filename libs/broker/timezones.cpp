#include "midas/timezones.hpp"
using namespace boost::posix_time;
using namespace boost::local_time;
using namespace boost::gregorian;
static boost::shared_ptr<dst_calc_rule> build_unified_us_ds_rules() {
  // rules for this zone are:
  // start on second Sunday of March at 2 am
  // end on first Sunday of November at 2 am
  // https://en.wikipedia.org/wiki/Energy_Policy_Act_of_2005#Change_to_daylight_saving_time
  nth_day_of_the_week_in_month start_rule(nth_day_of_the_week_in_month::second,
                                          Sunday, Mar);
  nth_day_of_the_week_in_month end_rule(nth_day_of_the_week_in_month::first,
                                        Sunday, Nov);

  return boost::shared_ptr<dst_calc_rule>(
      new nth_kday_dst_rule(start_rule, end_rule));
}
static const boost::shared_ptr<dst_calc_rule> us_ds_rules =
    build_unified_us_ds_rules();

static time_zone_ptr
build_tz(const std::pair<std::string, std::string> &standard_name,
         const std::pair<std::string, std::string> &daylight_savings_name,
         const time_duration &utc_offset,
         const dst_adjustment_offsets &dst_offset) {
  time_zone_names tz_names(standard_name.first, standard_name.second,
                           daylight_savings_name.first,
                           daylight_savings_name.second);
  return time_zone_ptr(
      new custom_time_zone(tz_names, utc_offset, dst_offset, us_ds_rules));
}

const boost::local_time::time_zone_ptr midas::eastern_us_tz = build_tz(
    {"America/New_York", "EST"}, {"America/New_York Daylight savings", "EDT"},
    time_duration(-5, 0, 0),
    dst_adjustment_offsets(time_duration(1, 0, 0), time_duration(2, 0, 0),
                           time_duration(2, 0, 0)));
const boost::local_time::time_zone_ptr midas::central_us_tz = build_tz(
    {"Central Time", "CST"}, {"Central Time Daylight savings", "CDT"},
    time_duration(-5, 0, 0),
    dst_adjustment_offsets(time_duration(-1, 0, 0), time_duration(2, 0, 0),
                           time_duration(2, 0, 0)));
