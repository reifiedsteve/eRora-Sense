#include "Chronos/Stopwatch.h"
#include "Diagnostics/Logging.h"

#include <list>

class Profiler
{
public:

    // TODO: string comparisons is too heavy and may distort timings.
    
    void start(const std::string& name) {
        _Timer& timer(_getTimer(name));
        timer.start();
    }

    void stop(const std::string& name) {
        _Timer& timer(_getTimer(name));
        timer.stop();
    }

    void report() {
        for (auto profile : _profiles) {
            Log.infoln("Profiler: time in \"%s\" is %d ms.", profile.name.c_str(), profile.timer.elapsed());
        }
    }

private:

    typedef Stopwatch _Timer;
    typedef _Timer::Ticks _Ticks;

    struct _Profile {
        std::string name;
        _Timer timer;
    };

    typedef std::list<_Profile> _Profiles;

    _Timer& _getTimer(const std::string& name) {
        
    }

    _Profiles _profiles;
};
