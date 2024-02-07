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

#include "Chronos/Stopwatch.h"

/// @brief A countdown timer.
/// Provides simple means of determining if a pre-determined countdown period has elapsed.
class CountdownTimer
{
public:

    typedef Stopwatch::Ticks Ticks;
    typedef Stopwatch::State State;

    // The default countdown period (if none is specified in the constructor).
    static const Ticks DefaultCountdownPeriod = 1000;

    /// @brief Constructor a countdown timer.
    /// @param countdown The countdown duration to be monitored.
    /// @param initially Begin the timer immediately if true, otherwise it begins in the stop state.
    inline CountdownTimer(Ticks countdown = DefaultCountdownPeriod, State initially = State::Stopped) __attribute__((always_inline))
      : _countdown(countdown)
      , _stopwatch()
    {
        if (initially == State::Running) _stopwatch.start();
    }

    /// @brief Start the countdown timer.
    inline void start() __attribute__((always_inline)) {
        _stopwatch.start();
    } 

    // Stop the countdown timer. 
    inline void stop() __attribute__((always_inline)) {
        _stopwatch.stop();
    }

    // Resets the countdown timer. (Does not alter its running state).
    inline void reset() __attribute__((always_inline)) {
        _stopwatch.reset();
    }

    /// @brief Restart the countdown timer. Equivalent to a reset of the timer then starting it.
    inline void restart() __attribute__((always_inline)) {
        _stopwatch.restart();
    }

    /// @brief Provides the percentage of time remaining. 
    /// @return The percentage of the countdown time remaining (0-100).
    inline uint8_t percentElapsed() const {
        return (uint8_t)((_countdown == 0) ? 100 : (100 * _elapsed() / _countdown));
    }

    /// @brief Provides the percentage of time elapsed. 
    /// @return The percentage of the countdown time elapsed (0-100).
    inline uint8_t percentRemaining() const {
        return (uint8_t)((_countdown == 0) ? 0 : (100 * _remaining() / _countdown));
    }

    /// @brief Provides the percentage of time remaining. 
    /// @return The percentage of the countdown time remaining (0.0-1.0).
    inline double fractionElapsed() const {
        Ticks elapsed(_elapsed());
        return (_countdown == 0) ? 1.0 : (1.0 * elapsed / _countdown);
    }

    /// @brief Provides the percentage of time elapsed. 
    /// @return The percentage of the countdown time elapsed (0-100).
    inline double fractionRemaining() const {
        return (_countdown == 0) ? 0.0 : (1.0 * _remaining() / _countdown);
    }

    /// @brief Determines if the countdown has expired.
    /// @return Returns true if the countdown duration has elapsed, otherwise returns false.
    inline bool hasExpired() const __attribute__((always_inline)) {
        return _stopwatch.elapsed() >= _countdown;
    }

    /// @brief Sets the countdown duration.
    /// @param countdown The countdown duration (in milliseconds).
    inline void setCountdown(Ticks countdown) __attribute__((always_inline)) {
        _countdown = countdown;
    }

    /// @brief Gets the countdown duration.
    /// @return The countdown duration.
    inline Ticks getCountdown() const __attribute__((always_inline)) {
        return _countdown;
    }

    /// @brief Determines the elapsed time until the countdown expires.
    /// @return The elapsed time (in milliseconds). Note: never exceeds the countdown time.
    inline Ticks elapsed() const __attribute__((always_inline)) {
        return _elapsed();
    }

    /// @brief Determines the remaining time until the countdown expires.
    /// @return The remaining time (in milliseconds).
    inline Ticks remaining() const __attribute__((always_inline)) {
        return _remaining();
    }

    /// @brief Inquires on whether the countdown timer is currently running or not.
    /// @return Returns true if running, otherwise false.
    inline bool isRunning() const  __attribute__((always_inline)) {
        return _stopwatch.isRunning();
    }

    /// @brief Immediately puts the countdown timer into an expired state.
    inline void expire() __attribute__((always_inline)) {
        _stopwatch.advanceBy(_remaining());
    }

private:

    Ticks _elapsed() const {
        return _countdown - remaining();
    }

    Ticks _remaining() const {
        Ticks elapsed(_stopwatch.elapsed());
        return (elapsed >= _countdown) ? 0 : (_countdown - elapsed);
    }

    Ticks _countdown;
    Stopwatch _stopwatch;
};
