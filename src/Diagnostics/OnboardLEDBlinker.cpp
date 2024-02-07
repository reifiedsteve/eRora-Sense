#include "Diagnostics/OnboardLEDBlinker.h"

void OnboardLEDBlinker::start(unsigned numBlinks) {
    _start(numBlinks, _onTimeMs, _offTimeMs); 
}

void OnboardLEDBlinker::start(unsigned numBlinks, int onOffTimeMs) {
    _start(numBlinks, onOffTimeMs, onOffTimeMs); 
}

void OnboardLEDBlinker::start(unsigned numBlinks, int onTimeMs, int offTimeMs) {
    _start(numBlinks, onTimeMs, offTimeMs); 
}

void OnboardLEDBlinker::loop()
{
    switch (_state)
    {
        case _State::On:

            if (_onTimer.hasExpired())
            {
                if (--_blinksToGo == 0) {
                    _goToIdleState();
                }
                
                else {
                    _goToOffState();
                }
            }
        
            break;

        case _State::Off:

            if (_offTimer.hasExpired()) {
                _goToOnState();
            }

            break;
        
        default: // also implicitly for Idle state.
            break;
    }
}

void OnboardLEDBlinker::_start(unsigned numBlinks, int onTimeMs, int offTimeMs)
{  
    _onTimer.setCountdown(onTimeMs);
    _offTimer.setCountdown(offTimeMs);

    _goToIdleState();

    if (numBlinks > 0)
    {
        _blinksToGo = numBlinks;

        // We begin in off state to ensure all on-blinks occur
        // as the blinker might be initiated from setup() and
        // there could be a few seconds until loop() is run by
        // which time the first timer will have expired (i.e.
        // an elongated initial off-state goes unnoticed and
        // is moot). Delaying the state change from off to on
        // does not compromise the on state in any way.

        _goToOffState(); 
    }
}

