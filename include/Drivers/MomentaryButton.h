#pragma once

#include <OneButton.h>
#include <functional>

class MomentaryButton
{
public:

    typedef std::function<void()> ButtonFunc;

    MomentaryButton() : _button() {
        _init();
    }

    explicit MomentaryButton(const int pin, const boolean activeLow = true, const bool pullupActive = true)
      : _button(pin, activeLow, pullupActive) 
    {
        _init();
    }

    inline void setDebounceMs(const unsigned int ms) {
        _button.setDebounceMs(ms);
    }

    inline void setClickMs(const unsigned int ms) {
        _button.setClickMs(ms);
    }

    inline void setPressMs(const unsigned int ms) {
        _button.setPressMs(ms);
    }

    inline void setLongPressIntervalMs(const unsigned int ms) {
        _button.setLongPressIntervalMs(ms);
    }

    inline void setIdleMs(const unsigned int ms) {
        _button.setIdleMs(ms);
    }

    inline void attachClick(ButtonFunc func) {
        _clickFunc = func;
    }

    inline void attachDoubleClick(ButtonFunc func) {
        _doubleClickFunc = func;
    }

    inline void attachMultiClick(ButtonFunc func) {
        _multiClickFunc = func;
    }

    inline void attachLongPressStart(ButtonFunc func) {
        _longPressStartFunc = func;
    }

    inline void attachDuringLongPress(ButtonFunc func) {
        _duringLongPressFunc = func;
    }

    inline void attachLongPressStop(ButtonFunc func) {
        _longPressStopFunc = func;
    }

    inline void attachIdle(ButtonFunc func) {
        _idleFunc = func;
    }

    void tick() {
        _button.tick();
    }

    void tick(bool activeLevel) {
        _button.tick(activeLevel);
    }

    void reset() {
        _button.reset();
    }

    int getNumberClicks() {
        return _button.getNumberClicks();
    }

    inline bool isIdle() const {
        return _button.isIdle();
    }

    inline bool isLongPressed() const {
        return _button.isLongPressed();
    }

private:

    void _init() {
        _button.attachClick([](void *scope) { ((MomentaryButton *) scope)->_onClicked();}, this);
        _button.attachDoubleClick([](void *scope) { ((MomentaryButton *) scope)->_onDoubleClicked();}, this);
        _button.attachMultiClick([](void *scope) { ((MomentaryButton *) scope)->_onMultiClick();}, this);
        _button.attachLongPressStart([](void *scope) { ((MomentaryButton *) scope)->_onLongPressStart();}, this);
        _button.attachDuringLongPress([](void *scope) { ((MomentaryButton *) scope)->_onDuringLongPress();}, this);
        _button.attachLongPressStop([](void *scope) { ((MomentaryButton *) scope)->_onLongPressStop();}, this);
    }

    void _onClicked() {
        if (_clickFunc) _clickFunc();
    }

    void _onDoubleClicked() {
        if (_doubleClickFunc) _doubleClickFunc();
    }

    void _onMultiClick() {
        if (_multiClickFunc) _multiClickFunc();
    }

    void _onLongPressStart() {
        if (_longPressStartFunc) _longPressStartFunc();
    }

    void _onDuringLongPress() {
        if (_duringLongPressFunc) _duringLongPressFunc();
    }

    void _onLongPressStop() {
        if (_longPressStopFunc) _longPressStopFunc();
    }

    void _onIdle() {
        if (_idleFunc) _idleFunc();
    }

    OneButton _button;
    ButtonFunc _clickFunc, _doubleClickFunc, _multiClickFunc, _longPressStartFunc, _duringLongPressFunc, _longPressStopFunc, _idleFunc;
};
