#pragma once

#include <Arduino.h>

class LimitedTrace
{
public:

    LimitedTrace(int maxNoOfReports)
      : _maxNoOfReports(maxNoOfReports)
      , _noOfReports(maxNoOfReports)
    {}

    void trace(const char* str) {
        if (_noOfReports > 0) {
            Serial.printf("At %s", str);
            Serial.println("");
        }
    }

    void incReport() {
      if (_noOfReports > 0) --_noOfReports;
    }

private:

    int _maxNoOfReports;
    int _noOfReports;
};