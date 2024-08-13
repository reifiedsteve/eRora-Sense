#pragma once

// #define SMART_SENSOR_USES_PMS7003

#include <list>

#include "SmartSensorObserver.h"

#include "Sensors/BME680Sensor.h"

#ifdef SMART_SENSOR_USES_PMS7003
#include "Sensors/PMS7003ParticleSensor.h"
#endif

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

#include "Fan/FanController.h"
#include "CabinetLights.h"

#include "DeviceState.h"
#include "CabinetLights.h"

#include "Diagnostics/HeapProfiler.h"
#include "Diagnostics/FrameRateProfiler.h"
class SmartSensor
{
public:

    typedef SmartSensorObserver Observer;
    
    static const TimeSpan DefaultInterval;

    static const int MinFanSpeed = 1;
    static const int MaxFanSpeed = 10;

    SmartSensor();
    
    void bindObserver(Observer& observer);

    void bindFanController(FanController& controller);
    void bindCabinetLights(CabinetLights& cabinetLights);

    // void bindFanSpeedObserver();

    void switchPower(bool on);
    void togglePower();

    void selectNextDisplayMode();

    /// @brief Set the fan speed (for when fan is in manual mode).
    /// @param speedSetting Setting of 0-10 (off to maximum). 
    /// (Values outside of 1-10 will be constrained to 1-10.)
    void setFanSpeed(int speedSetting);

    /// @brief Adjust the fan speed (for when fan is in manual mode).
    /// @param speedSetting Amount by which to adjust the fan speed.
    void adjustFanSpeed(int delta);

    /// @brief Trigger extra illumination for inspection purposes.
    void triggerInspectionLight();

#if 0

    inline bool isSensorReady() const {
        return _state.sensorReady;
    }

    inline float getTemperature() const {
        return _state.temperature;
    }

    inline float getHumidity() const {
        return _state.relHumidity;
    }

    inline float getAirPressure() const {
        return _state.airPressure;
    }

    inline float getIAQ() const {
        return _state.iaq;
    }

    inline float getTVOC() const {
        return _state.tvoc;
    }

    inline float getCO2() const {
        return _state.co2;
    }

    inline float getPM01() const {
        return _state.pm01;
    }

    inline float getPM25() const {
        return _state.pm25;
    }

    inline float getPM10() const {
        return _state.pm10;
    }

#endif

   /// @brief Reboot the multi-sensor.
    void reboot();

    bool isOn() const;
    int getFanSpeed() const;

    void setup();
    void loop();

private:

    #ifdef SMARTSENSOR_IS_MUTEX_GUARDED
    typedef std::mutex _Mutex;
    typedef std::lock_guard<_Mutex> _ScopedLock;
    #else
    class _Mutex { public: inline void lock() {}  inline bool try_lock() { return true; } inline void unlock() {} };
    class _ScopedLock { public: inline _ScopedLock(_Mutex& mutex) {} };
    #endif

    void _doSetFanSpeed(int speedSetting);
    void _doAdjustFanSpeed(int delta);

    bool _setPower(bool on);
    bool _setFanSpeed(int speedSetting);

    void _triggerInspectionLight();

    void _processTemperature(float temperature);
    void _processHumidity(float relHumidity);
    void _processAirPressure(float airPressure);
    void _processTVOC(float tvoc);
    void _processCO2(float co2);
    void _processIAQ(bool available, float iaq);

    void _processPM01(uint16_t pm01);
    void _processPM25(uint16_t pm25);
    void _processPM10(uint16_t pm10);

    void _informOfTemperature(float temperature);
    void _informOfHumidity(float humidity);
    void _informOfAirPressure(float humidity);
    void _informOfTVOC(float tvoc);
    void _informOfCO2(float co2);
    void _informOfIAQ(float iaq);

    void _informOfPM01(uint16_t pm01);
    void _informOfPM25(uint16_t pm25);
    void _informOfPM10(uint16_t pm10);

    void _informOfIAQAvailability(bool available);

    void _informOfPower(bool on);
    void _informOfFanSpeed(int fanSpeed);
    void _informOfInspectionLight(bool onOff);

    void _informOfHeapUsage(uint32_t totalHeap, uint32_t freeHeap);
    void _informOfFPS(unsigned fps);
    
    static int _constrainFanSpeed(int speed);

    typedef std::list<Observer*> _Observers;

    BME680Sensor _bmeSensor;

    #ifdef SMART_SENSOR_USES_PMS7003
    PMS7003ParticleSensor _pmSensor;
    #endif

    DeviceState _state;

    CountdownTimer _timer;
    _Observers _observers;

    FanController* _fan;

    CabinetLights* _cabinetLights;

    HeapProfiler _heapProfiler;
    FrameRateProfiler _fpsProfiler;

    mutable _Mutex _mutex;
};

