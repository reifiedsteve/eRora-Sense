#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm> // .cpp

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

/// @brief A base class for presenting a character matrix, such as 
/// a standardised means to drive an LCD display. Provides
/// auto-scrolling when a line of text is too long for the device.
/// Also (TOD) supports other character attributes such as flashing.
class CharacterMatrix
{
public:

    CharacterMatrix(size_t width, size_t height, const TimeSpan& refreshInterval = TimeSpan::fromSeconds(1));

    void clear();

    inline void writeLine(int lineNo, const std::string& text) __attribute__((always_inline)) {
        // check: too long for line? Truncate or (if autoscroll) init scrolling?
        _setLine(lineNo, text);
    }

    inline int width() const __attribute__((always_inline)) {
        return _width;
    }
    
    inline int height() const __attribute__((always_inline)) {
        return _height;
    }
    
    void show();

protected:

    /// @brief Write some text to a character matrix.
    /// @param y The row number, based from zero. Zero is at the top.
    /// @param x The column number, based from zero. Zero is on the left.
    /// @param str The text to be placed at the specified location.
    virtual void _writeText(int y, int x, const std::string& str) = 0;

    /// @brief Write a character of a character matrix.
    /// @param y The row number, based from zero. Zero is at the top.
    /// @param x The column number, based from zero. Zero is on the left.
    /// @param ch The character to be placed at the specified location.
    virtual void _writeChar(int y, int x, char ch) = 0;

    /// @brief Refresh the display to show the current character matrix
    /// display, as per any written characters. If the device is buffered
    /// then this ensures all recent charater-write are shown.
    virtual void _refresh() = 0;

private:

    typedef std::vector<std::string> _Lines;

    void _init(size_t width, size_t height);

    void _clear();
    void _setLine(int lineNo, const std::string& text);

    void _writeLine(int lineNo, const std::string& text);

    size_t _width, _height;
    _Lines _lines;
    bool _dirty;

    CountdownTimer _timer;
};
