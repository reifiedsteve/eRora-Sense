
#include "Chronos/Clock.h"
#include "Diagnostics/Logging.h"

#include <ctime>
#include <cstdlib>

// Timezones info: https://www.timeanddate.com/time/zones/

TimeSpan Clock::PointInTime::operator-(const Clock::PointInTime& rhs) const {
    const std::tm& tm1(*this);
    const std::tm& tm2(rhs);
    time_t start(std::mktime((const_cast<std::tm*>(&tm1)))); // mktime should take const arg!
    time_t end(std::mktime(const_cast<std::tm*>(&tm2)));
    long int interval(difftime(end, start));
    return TimeSpan::fromSeconds(interval);
}

Clock::Clock(const std::string& ntpServer, const std::string& tz)
    : _ntpServer(ntpServer)
    , _tz(tz)
    , _timeout(1)
    , _gmtOffsetSecs(0)
    , _daylightOffsetSecs(3600)
{}

void Clock::_initTimeNTP()
{
    _setTimeZone();

    // There is a list of NTP servers at:
    // https://timetoolsltd.com/information/public-ntp-server/
    // Includes advice on how to select a relevant one.
    // Server at pool.ntp.org is a default, which should select
    // a localish one, but can be explicit using e.g.
    // europe.pool.ntp.org, or more locally, uk.pool.ntp.org.
    // See also: https://support.ntp.org/Servers/NTPPoolServers
    // for advice on server selection. Generally using pool.ntp.org 
    // will choose a local one for you.

    configTime(_gmtOffsetSecs, _daylightOffsetSecs, _ntpServer.c_str(), nullptr, nullptr);

    #if 0

    // TODO: Implement timezones!
    // Diagnostics: Probably too early to log time as may not have connection yet with NTP server.
    // USeful links:
    // https://stackoverflow.com/questions/61110731/esp32-setenv-and-localtime-from-utc-time-string
    // https://en.wikipedia.org/wiki/Tz_database

    std::string now;
    _tryGetTimeFormatted("%a %F %H:%M", now);
    Log.verboseln("Clock: initialised. Currently %s.", now.c_str()); 

    #endif
}

void Clock::_setTimeZone() {
    setenv("TZ", _tz.c_str(), 1);
    // setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1); // UK hardcoded for now. TODO: need a TZ DB, selection via Web-UI (installation) etc.
    tzset();
}

bool Clock::_tryGetTimeFormatted(const char* format, std::string& result)
{
    bool got(false);

    // NOTE: If this times out, then probably not connected to the internet.
    // (Did you remember to do that? You need to for access to an NTP server!)

    PointInTime now;

    if (_tryGetPointInTime(now)) {
        char buf[32];
        std::size_t n = strftime(buf, sizeof(buf), format, &now);
        buf[n] = 0; // Needed?
        result = std::string(buf);
        got = true;
    }

    return got;
}

bool Clock::_tryGetPointInTime(PointInTime& when)
{
    bool got(false);
    PointInTime now;

    // NOTE: If this times out, then probably not connected to the internet.
    // (Did you remember to do that? You need to for access to an NTP server!)

    #if 0

    // TODO: test this then replace getLocalTime() use with it!
    time_t nowTime(std::time(nullptr));
    std::tm* nowTm = std::localtime(&nowTime);
    if (nowTm) {
        now = *nowTm;
        when = now;
        got = true;
    }

    #else

    if (getLocalTime(&now, _timeout)) {     // TODO: Note: getLocalTime *may* not respect timezones/dst etc. use localtime().
        when = now;
        got = true;
    }

    #endif

    return got;
}
