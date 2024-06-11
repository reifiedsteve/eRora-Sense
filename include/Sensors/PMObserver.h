#pragma once

#include <inttypes.h>

class PMObserver
{
public:

    virtual void onPM01(uint16_t pm01) = 0;
    virtual void onPM25(uint16_t pm25) = 0;
    virtual void onPM10(uint16_t pm10) = 0;
};
