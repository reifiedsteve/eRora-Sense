#pragma once

// ####################################################################################
// 
//                              `_`     `_,_`  _'                                  `,
//                             -#@@- >O#@@@@u B@@>                                 8@E
//     :)ilc}` `=|}uccccVu}r"   VQz `@@#Mhzk= |8M   `=v}ucccccuY),    `~v}uVVcccccV#@$
//   ^Q@#EMqK.I#@QRdMqqMdRQ@@Q, Q@B `@@BqqqW^ W@@` e@@QRdMMMMbEQ@@8: i#@BOMqqqqqqqM#@$
//   D@@`    )@@x          <@@T Q@B `@@q      W@@`>@@l          :@@z`#@d           Q@$
//   D@#     ?@@@##########@@@} Q@B `@@q      W@@`^@@@##########@@@y`#@W           Q@$
//   0@#     )@@d!::::::::::::` Q@B `@@M      W@@`<@@E!::::::::::::``#@b          `B@$
//   D@#     `m@@#bGPP}         Q@B `@@q      W@@` 3@@BbPPPV         y@@QZPPPPPGME#@8=
//   *yx       .*icywwv         )yv  }y>      ~yT   .^icywyL          .*]uywwwwycL^-
// 
//      (c) 2022 Reified Ltd.     W: www.reified.co.uk     E: info@reified.co.uk
// 
// ####################################################################################

#include <Arduino.h>

/// @brief A stopwatch class providing measurements of elapsed (wall-clock) time.
class Stopwatch
{
public:

    /// @brief Ticks, in milliseconds, providing an 8-byte range.
    typedef uint64_t Ticks;

    static const Ticks TicksPerSecond = 1000;
    
    /// @brief The running state of the stopwtach: either stopped or running.
    enum class State : uint8_t {
        /// @brief Stopwatch state of stopped.
        Stopped,
        /// @brief Stopwatch state of running.
        Running
    };

    /// @brief Stopwatch constructor
    /// @param intially The initial state of the stopwatch.
    explicit Stopwatch(State intially = State::Stopped);

    /// @brief Start the stopwatch. If already running then it has no effect.
    inline void start() __attribute__((always_inline));

    /// @brief Stop the stopwatch. If already stopped then it has no effect.
    inline void stop() __attribute__((always_inline));

    /// @brief Reset the stopwatch (to an elapsed time of zero). Note: this has no effect on the running state.
    inline void reset() __attribute__((always_inline));

    /// @brief Restart the stopwatch. Equivalent to resetting then starting the stopwatch.
    inline void restart();
    
    /// @brief Provide the elapsed time measured by the stopwatch.
    /// @return The elapsed time.
    inline Ticks elapsed() const __attribute__((always_inline)); // 64-bit, milliseconds, so better than ::millis() as no 32-bit overflow.  :)

    /// @brief Provide the elapsed time as component parts.
    /// @param hours  The returned component hours.
    /// @param minutes The returned component minutes.
    /// @param seconds The returned component seconds.
    /// @param millis The returned component milliseconds.
    void elapsed(uint32_t& hours, uint8_t& minutes, uint8_t& seconds, uint16_t& millis);

    /// @brief Advance the stopwatch by the given amount.
    /// @param delta The amount by which to advance the stopwatch.
    inline void advanceBy(Ticks delta) __attribute__((always_inline));

    /// @brief Inquires on whether the stopwatch is currently running or not.
    /// @return Returns true if running, otherwise false.
    inline bool isRunning() const __attribute__((always_inline));

private:

    typedef struct {
        uint32_t overflows;
        uint32_t lastTimeSeen;
    } _ElapsedEver;

    void _start();
    void _stop();

    void _reset();

    Ticks _elapsed() const;

    inline Ticks _millis() const __attribute__((always_inline)); // 64-bit, so better than ::millis() as no 32-bit overflow.  :)
    inline Ticks _ticks() const __attribute__((always_inline)); // 64-bit, so better than ::millis() as no 32-bit overflow.  :)

    Ticks _startTime;
    Ticks _stopTime;

    mutable _ElapsedEver _elapsedEver; // Making this static would mean the need for mutex protection, which might cause performsnce issues if called very often as is common with typical code that calls ::millis().

    Ticks _totalPauseTime;

    bool _running;
    bool _doReset;
};

inline void Stopwatch::start() {
    _start();
}

inline void Stopwatch::stop() {
    _stop();
}

inline void Stopwatch::reset() {
    _reset();
}

inline void Stopwatch::restart() {
    _stop();
    _reset();
    _start();
}

inline Stopwatch::Ticks Stopwatch::elapsed() const {
    return _elapsed();
}

inline void Stopwatch::advanceBy(Ticks delta) {
    _startTime -= delta;
}

inline bool Stopwatch::isRunning() const {
    return _running;
}

inline Stopwatch::Ticks Stopwatch::_ticks() const {
    return esp_timer_get_time(); // FreeRTOS esp_timer_get_time() is micro-secs with 200+ year rollover period.
}



