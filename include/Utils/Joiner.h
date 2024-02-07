#pragma once

#include <istream>
#include <limits>

class Joiner
{
public:

    template <typename _Iterator>
    static std::string join(_Iterator begin, _Iterator end, char delimiter) {
        std::stringstream os;
        join(os, begin, end, delimiter);
        return os.str();
    }

    template <typename _Iterator>
    static void join(std::ostream& os, _Iterator begin, _Iterator end, char delimiter)
    {
        _Iterator iter(begin);

        if (iter != end) {
            os << *iter;
            while (++iter != end) {
                os << delimiter;
                os << *iter;
            }
        }
    }
};

