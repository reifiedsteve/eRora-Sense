#pragma once

#include <istream>
#include <limits>

class Splitter
{
public:

    template <typename _Inserter>
    static int split(std::string str, _Inserter inserter, char delimiter, bool ignoreEmpty = true, unsigned maxItems = std::numeric_limits<unsigned>::max()) {
        std::istringstream is(str);
        return split(is, inserter, delimiter, ignoreEmpty, maxItems);
    }

    template <typename _Inserter>
    static int split(std::istream& is, _Inserter inserter, char delimiter, bool ignoreEmpty = true, unsigned maxItems = std::numeric_limits<unsigned>::max()) 
    {
        int n = 0;

        std::string part;

        while ((n < maxItems) && std::getline(is, part, delimiter)) {
            if (!ignoreEmpty || part.length() > 0) {
                *inserter++ = part;
                ++n;
            }
        }

        return n;
    }
};

