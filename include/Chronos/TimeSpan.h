#pragma once

#include <inttypes.h>
#include "Diagnostics/Logging.h"

class TimeSpan
{
public:

    typedef uint64_t Ticks;

    enum class Units : uint8_t {
        Ticks, /* means highest possible resolution */
        Microseconds,
        Milliseconds,
        Seconds,
        Minutes
    };

    static const TimeSpan Zero;
    
    inline static TimeSpan fromMicroseconds(Ticks ms) __attribute__((always_inline)) {
        return TimeSpan(ms, Units::Microseconds);
    }

    inline static TimeSpan fromMilliseconds(Ticks ms) __attribute__((always_inline)) {
        return TimeSpan(ms, Units::Milliseconds);
    }

    inline static TimeSpan fromSeconds(Ticks ms) __attribute__((always_inline)) {
        return TimeSpan(ms, Units::Seconds);
    }

    inline static TimeSpan fromMinutess(Ticks ms) __attribute__((always_inline)) {
        return TimeSpan(ms, Units::Minutes);
    }

    /// @brief Construct the time span. Note that units must be explicitly provided 
    /// in order to eliminate assumptions (and associated bugs).
    /// @param span The time span numerically.
    /// @param units THe units for that time span value.
    inline explicit TimeSpan(Ticks span, Units units) __attribute__((always_inline)) {
        _set(span, units);
    }

    /// @brief Set the time span. Note that units must be explicitly provided.
    /// @param span The time span numerically.
    /// @param units THe units for that time span value.
    inline void set(Ticks span, Units units) __attribute__((always_inline)) {
        _set(span, units);
    }

    /// @brief Get the timespan in the explicitly provided units.
    /// @param units The required units for the time span.
    inline Ticks get(Units units) const __attribute__((always_inline)) {
        return _get(units);
    }

    /// @brief Get the timespan in milliseconds.
    /// @return The number of elapsed milliseconds.
    inline Ticks millis() const __attribute__((always_inline)) {
        return _ticks / _ticksPerMillisecond;
    }

    /// @brief Get the timespan in ticks.
    /// @return The number of elapsed ticks.
    inline Ticks ticks() const  __attribute__((always_inline)) {
        return _ticks;
    }

    /// @brief Set the time span. Note that units must be explicitly provided.
    /// @param span The time span numerically.
    /// @param units THe units for that time span value.
    inline void setPrecise(double span, Units units) __attribute__((always_inline)) {
        _setPrecise(span, units);
    }

    /// @brief Get the timespan in the explicitly provided units.
    /// @param units The required units for the time span.
    inline double getPrecise(Units units) const __attribute__((always_inline)) {
        return _getPrecise(units);
    }

    inline bool operator==(const TimeSpan& rhs) const {
        return _ticks == rhs._ticks;
    }

    inline bool operator!=(const TimeSpan& rhs) const {
        return _ticks != rhs._ticks;
    }

    inline bool operator<(const TimeSpan& rhs) const {
        return _ticks < rhs._ticks;
    }

    inline bool operator>(const TimeSpan& rhs) const {
        return _ticks > rhs._ticks;
    }

    inline bool operator<=(const TimeSpan& rhs) const {
        return _ticks <= rhs._ticks;
    }

    inline bool operator>=(const TimeSpan& rhs) const {
        return _ticks >= rhs._ticks;
    }

private:

    void _set(Ticks span, Units units);
    void _setPrecise(double span, Units units);

    Ticks _get(Units units) const;
    double _getPrecise(Units units) const;

    static const Ticks _ticksPerMillisecond = 1000;
    static const Ticks _ticksPerSecond = 1000000;
    static const Ticks _secondsPerMinute = 60;

    Ticks _ticks; // uSecs.
};
