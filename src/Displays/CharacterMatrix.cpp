#include "Displays/CharacterMatrix.h"

CharacterMatrix::CharacterMatrix(size_t width, size_t height, const TimeSpan& refreshInterval)
    : _width(width)
    , _height(height)
    , _lines()
    , _dirty(false)
    , _timer(refreshInterval.millis(), CountdownTimer::State::Running)
{
    _init(width, height);
    _clear();
}

void CharacterMatrix::clear() {
    _clear();
}

void CharacterMatrix::show()
{
    // No need to flood the display device. 
    // Just send at a user-defined appropriate rate.

    if (_dirty && _timer.hasExpired())
    {
        for (int y = 0; y < _height; ++y) {
            _writeLine(y, _lines[y]);
        }

        _refresh();

        _dirty = false;
    }
}

void CharacterMatrix::_init(size_t width, size_t height) {
    _lines.reserve(height);
    for (int y = 0; y < height; ++y) {
        _lines.push_back(std::string(width, ' '));
    }
}

void CharacterMatrix::_clear() {
    for(std::string& line : _lines) {
        for(char& ch : line) {
            ch = ' ';
        }
    }
    _dirty = true;
}

void CharacterMatrix::_setLine(int lineNo, const std::string& text)
{
    size_t len(text.size());
    len = ((len < _width) ? len : _width);

    for (int x = 0; x < len; ++x) {
        _lines[lineNo][x] = text[x];
    }

    for (int x = len; x < _width; ++x) {
        _lines[lineNo][x] = ' ';
    }

    _dirty = true;

    // Log.verboseln("Line %d, setting text to \"%s\"", lineNo, _lines[lineNo].c_str());
}

void CharacterMatrix::_writeLine(int lineNo, const std::string& text) {
    _writeText(lineNo, 0, text);
}
