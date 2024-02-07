#pragma once

#include <inttypes.h>
#include "Diagnostics/Logging.h"
#include <sstream>
#include <iomanip>

class HexTable
{
public:

    HexTable() 
      : _columns(16)
      , _offset(0)
      , _colNo(0)
      , _os()
    {}

    static void show(const uint8_t* bytes, unsigned count) {
        HexTable table;
        table.put(bytes, count);
        table.end();
    }

    void put(const uint8_t* bytes, unsigned count) {
        const uint8_t* ptr = bytes;
        for (unsigned i = 0; i < count; ++i) {
            _put(*ptr++);
        }
    }

    void put(uint8_t val) {
        _put(val);
    }

    void end() {
        _os << std::endl;
        Log.verboseln("\n%s", _os.str().c_str());
        _os.clear();
    }

private:

    void _put(uint8_t val) {
        if (_colNo == _columns) {
            _os << std::endl;
            Log.verboseln("%s", _os.str().c_str());
            _os.clear();
            _colNo = 0;
        }
        if ((_offset % _columns) == 0) {
            _os << std::setw(8) << _offset << ": ";
        }
        _os << std::setw(3) << (unsigned) val << " ";
        ++_offset;
        ++_colNo;
    }

    unsigned _columns;
    unsigned _offset;
    unsigned _colNo;

    std::ostringstream _os;
};
