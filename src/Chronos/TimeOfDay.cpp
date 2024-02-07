#include "Chronos/TimeOfDay.h"
#include "Utils/Parser.h"

bool TimeOfDay::tryParse(const std::string& str, TimeOfDay& time)
{
    bool parsed(false);

    if ((str.size() == 5) && (str[2] == ':')) 
    {
        int hour, minute;

#if false

#else
        if (Parser::tryParse(str.substr(0, 2), hour) && Parser::tryParse(str.substr(3, 2), minute) && (0 <= hour) && (hour <= 23) && (0 <= minute) && (minute <= 59)) {
            time = TimeOfDay(hour, minute);
            parsed = true;
            // Log.verboseln("TimeOfDay: parsed \"%s\" - hour is %d, minute is %d.", str.c_str(), hour, minute);
        }
#endif

    }

    if (!parsed) {
        Log.verboseln("TimeOfDay: failed to parse \"%s\".", str.c_str());
    }
    return parsed;
}
