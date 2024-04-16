#pragma once

#include <Arduino.h>
#include <PMserial.h>

#include "Chronos/TimeSpan.h"
#include "Chronos/CountdownTimer.h"

class ParticleSensor
{
public:

    static const TimeSpan DefaultInterval; // = TimeSpan(10, TimeSpan::Units::Minutes)

    struct Measurements {
        Measurements() : pm01(0), pm25(0), pm10(0) {}
        uint16_t pm01; // PM 1.0
        uint16_t pm25; // PM 2.5
        uint16_t pm10; // PM 10.0
    };

    /// @brief Construct a particle sensor.
    /// Running the hardware continuously will
    /// result in the sensor expiring after around 8 months,
    /// so we only sample occasionally (defined by interval)
    /// and put the sensor to sleep between taking samples.
    /// @param rxPin ESP32 RX pin connected to the PMS TX out.
    /// @param txPin ESP32 TX pin connected to the PMS RX in.
    /// @param interval How often to sample PM data
    explicit ParticleSensor(
        uint8_t rxPin = 16,
        uint8_t txPin = 17,
        const TimeSpan& interval = DefaultInterval
    );

    /// @brief Initialise the sensor.
    void setup();

    /// @brief  Inquire on whether or not a new sample is available.
    /// @return True if available; otherwise false.
    bool available();

    /// @brief Read the most recent sample.
    /// @param pm01 Level of PM 1.0 particulate matter.
    /// @param pm25 Level of PM 2.5 particulate matter.
    /// @param pm10 Level of PM 10.0 particulate matter.
    const Measurements& read();
 
private:

    enum class _State {
        Init,
        Sleeping,
        Reading
    };

    bool _cycle();

    void _cycleInit();
    void _cycleSleeping();
    bool _cycleReading();

    SerialPM _pms;
    _State _state;

    bool _available;
    
    Measurements _measurements;

    CountdownTimer _timer;
};
