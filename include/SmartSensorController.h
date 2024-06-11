#pragma once

#include "SmartSensor.h"

#include "Chronos/Stopwatch.h"

#include <vector>

// This will ensure that any callbacks from any controller give back
// control to that controller in minimal time as we merely queue
// the callback for (slightly) later execution here which will
// be on our our thread and not that of the controller. 
// This has two main benefits:
// (a) returns control to the controller as quickly as possible.
// (b) ensures synchronicity between control of and executing animations.
// It has one disadvantage: it is more code (and code-space is tight).
// However, it has a major benefit of avoiding possible re-entrancy issues,
// thus reducing the number of considerations for (and complexity of)
// other code.

#define OPERATIONS_EXECUTE_ON_MAIN_THREAD

#ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
#include <queue>
#include <mutex>
#endif

class SmartSensorController
{
public:

    typedef SmartSensorController This;
 
    typedef CountdownTimer::Ticks Ticks;
 
    enum class Responsiveness : uint8_t {
        WhenOn,
        Always
    };

    /// @brief Returns the descriptive name of the controller.
    /// @return The name.
    const char* controllerName() const;

    /// @brief Set how often the controller runs.
    /// @param ticks The interval (in ms) between iterations.
    void setLoopInterval(Ticks ticks);

    /// @brief Get how oftne the controller runs.
    /// @return The interval (in ms) between iterations.
    Ticks getLoopInterval() const;

    /// @brief Inquire whether this controller should be reactive when the light is turned off.
    /// @return If response then returns true, otherwise false.
    bool responsiveWhenOff() const;

    /// @brief Initialised the controller.
    void setup();

    /// @brief Executes a single cycle of the controller.
    void loop();

protected:

    SmartSensorController(SmartSensor& sensor, const char* controllerName, const TimeSpan& executionInterval, Responsiveness responsiveness);

    virtual void _initInputs() = 0;
    virtual void _serviceInputs() = 0;

    // Powering on or off refers to the fan/cabinet-backlight
    // (and maybe the LCD backlight).
    // The sensors continue to sample and publish to all
    // observers regardless.

    void _switchOnOff(bool on); // on/off of backlight and fan.
    void _toggleOnOff();

    /// @brief Set the fan to speed 0-10.
    /// @param speed teh fan speed, 0 being still, 1 being minimum, 10 being full speed
    void _setFanSpeed(uint8_t speed);

    /// @brief Adjust the fan speed.
    /// @param delta The amount by which to change the fan speed.
    void _adjustFanSpeed(int8_t delta);

    // Set the cabinet backlight brightness, 0 being minimum,
    // 255 being full brightness.
    void _setBacklightBrightness(uint8_t brightness);
    void _setBacklightColour(uint8_t hue, uint8_t sat);

    // void _setDisplayMode(DisplayMode mode); // TODO:
    void _selectNextDisplayMode();
    
    void _reboot();

    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD    
    typedef std::function<void()> _DeferredOperation;
    void _scheduleOperation(_DeferredOperation op);
    void _executeDeferredOperations();
    #endif

    static bool _allDigits(const std::string& str);

    static bool _meansTrue(const std::string& str);
    static bool _meansFalse(const std::string& str);

private:

    typedef std::mutex _Mutex;
    typedef std::lock_guard<_Mutex> _ScopedLock;

    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    typedef std::queue<_DeferredOperation> _DeferredOperations;
    #endif

    SmartSensorController(const SmartSensorController& rhs) = delete;
    SmartSensorController& operator=(const SmartSensorController& rhs) = delete;

    // Internal state.

    SmartSensor& _smartSensor;
    const char* _controllerName;

    Responsiveness _responsiveness;
    bool _responsiveWhenOff;

    CountdownTimer _controllerTimer;

    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    _Mutex _operationsMutex;
    _DeferredOperations _deferredOperations;
    #endif
};