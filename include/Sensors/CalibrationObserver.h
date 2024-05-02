#pragma once

#include "CalibrationState.h"
#include "Chronos/TimeSpan.h"

class CalibrationObserver
{
public:

    virtual void onCalibration(CalibrationState state, const TimeSpan& time) = 0;
};