#pragma once

#include "Chronos/DayOfWeek.h"

#include <string>
#include <ctime>

/// @brief Represents a wall clock for telling the current time and/or date.
class Clock
{
public:

    struct PointInTime : public std::tm
    {
        typedef ::DayOfWeek DayOfWeek;

        inline int day() const __attribute__((always_inline)) {
            return tm_mday;
        }

        inline int month() __attribute__((always_inline)) {
            return tm_mon + 1;
        }

        inline int year() __attribute__((always_inline)) {
            return 1900 + tm_year;
        }

        inline int hour() const __attribute__((always_inline)) {
            return tm_hour;
        }

        inline int minute() const __attribute__((always_inline)) {
            return tm_min;
        }

        inline int second() const __attribute__((always_inline)) {
            return tm_sec;
        }

        inline DayOfWeek dayOfWeek() const __attribute__((always_inline)) {
            return (DayOfWeek)tm_wday;
        }

        inline bool isDaylightSaving() const __attribute__((always_inline)) {
            return tm_isdst;
        }
    };

    /// @brief Construct a Clock object, optionally providing the name of 
    /// @brief an NTP server. Defaults to "pool.ntp.org" which should work
    /// @brief regardless. Also optionally provide a timezone definition.
    /// @param ntpServer THe host name of the NTP server.
    /// @param tx Timezone string 
    /// (see: https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.json)
    explicit Clock(const std::string& ntpServer = "pool.ntp.org", const std::string& tz = "GMT0");

    /// @brief Set the offset from GMT in seconds for your zone. Default is 0.
    /// @brief For timezones see: https://www.timeanddate.com/time/zones/
    /// @param seconds The offset in seconds.
    inline void setGmtOffset(long seconds) {
        _gmtOffsetSecs = seconds;
    }

    /// @brief Set the daylight saving time offset in seconds. 
    /// @brief Commonly 3600, hence the default is 3600.
    /// @param daylightSavingOffsetSeconds 
    inline void setDstOffset(long daylightSavingOffsetSeconds) {
        _daylightOffsetSecs = daylightSavingOffsetSeconds;
    }

    /// @brief Set the timeout for when waiting for the NTP server to respond.
    /// @brief The default is 1ms. 
    /// @brief (Warning: this can can several tens of milliseconds minimum
    /// @brief regardless of timeout settings.)
    /// @param ms 
    inline void setTimeout(uint32_t ms) {
        _timeout = ms;
    }

    /// @brief Perform any post-construction deferred initialisation.
    inline void setup() {
        _initTimeNTP();
    }

    /// @brief Try to get the current point in time.
    /// @param The returned point in time.
    /// @return True if now was returned, otherwise false.
    inline bool tryGetNow(PointInTime& now) {
        return _tryGetPointInTime(now);
    }

    /// @brief Retrieve the time in the simple "HH:MM" or "HH:MM:SS" format.
    /// @brief (Warning: this can can several tens of milliseconds minimum
    /// @brief regardless of timeout settings.)
    /// @param includeSeconds Specify true for "HH:MM:SS" format. Default is false.
    /// @return The formatted time, or the empty string for an NTP failure.
    inline std::string getTimeFormatted(bool includeSeconds = false) {
        std::string result("");
        _tryGetTimeFormatted(includeSeconds ? "%H:%M:%S" : "%H:%M", result);
        return result;
    }

    /// @brief Retrieve the date and time in universal form "DAY YYYY-MM-DD HH:MM".
    /// @brief (Warning: this can can several tens of milliseconds minimum
    /// @brief regardless of timeout settings.)
    /// @return The formatted date and time, or the empty string for an NTP failure.
    inline std::string getTimeAndDateFormatted() {
        std::string result("");
        _tryGetTimeFormatted("%a %F %H:%M", result);
        return result;
    }

    /// @brief Retrive date and/or time according to the specified format.
    /// @brief See https://cplusplus.com/reference/ctime/strftime/ for format options.
    /// @param format The format of the required result.
    /// @return The date and/or time formatted as specified.
    inline std::string getFormatted(const char* format) {
        std::string result("");
        _tryGetTimeFormatted(format, result);
        return result;
    }

private:

    /// @brief Initialise the use of an NTP server to be able to retrieve
    /// @brief an accurate representation of the current time.
    void _initTimeNTP();

    void _setTimeZone();

    /// @brief Get the current time as a formatted string. 
    /// @brief See https://cplusplus.com/reference/ctime/strftime/ for format options.
    /// @param format The format string.
    /// @return The formatted time.
    bool _tryGetTimeFormatted(const char* format, std::string& result);

    bool _tryGetPointInTime(PointInTime& when);

    std::string _ntpServer;
    std::string _tz;

    uint32_t _timeout; // ms.
    long _gmtOffsetSecs, _daylightOffsetSecs;
};
