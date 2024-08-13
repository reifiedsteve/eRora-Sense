#pragma once

#include <ios>

class StreamRestorer {
public:
    inline StreamRestorer(std::ios_base& ios)
        : _ios(ios), _flags(ios.flags())
    {}
    inline ~StreamRestorer() {
        _ios.flags(_flags);
    }
private:
    std::ios_base& _ios;
    std::ios_base::fmtflags _flags;
};

