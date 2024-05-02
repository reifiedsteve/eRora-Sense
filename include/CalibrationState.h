#pragma once

enum class CalibrationState {
    BurnIn,
    WarmUp,
    Ready,
    Expired
};

const char* renderCalibrationState(CalibrationState state) 
{
    const char* str = "?";

    switch (state) 
    {
        case CalibrationState::BurnIn:
            str = "Burn-In";
            break;
        case CalibrationState::WarmUp:
            str = "Warm-Up";
            break;
        case CalibrationState::Ready:
            str = "Ready";
            break;
        case CalibrationState::Expired:
            str = "Expired";
            break;
        default:
            break;            
    }

    return str;
};
