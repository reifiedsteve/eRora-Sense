#pragma once

#include <array>

class MACAddress
{
public:

    static MACAddress zero();
    static MACAddress local();

    inline MACAddress() 
      : _a(0), _b(0), _c(0), _d(0), _e(0), _f(0)
    {}

    inline MACAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
      : _a(a), _b(b), _c(c), _d(d), _e(e), _f(f)
    {}

    inline void _set(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) {
        _a = a; _b = b; _c = c; _d = d; _e = e; _f = f;
    }

    inline bool operator==(const MACAddress& rhs) const __attribute__((always_inline)) {
        return _same(rhs);
    }

    inline bool operator!=(const MACAddress& rhs) const __attribute__((always_inline)) {
        return !_same(rhs);
    }

    std::string str(const char* delimiter = "") const;

private:

    inline bool _same(const MACAddress& rhs) const {
        return (_a == rhs._a) && (_b == rhs._b) && (_c == rhs._c) && (_d == rhs._d) && (_e == rhs._e) && (_f == rhs._f);
    }

    uint8_t _a, _b, _c, _d, _e, _f;
};
