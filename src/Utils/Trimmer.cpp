#include "Utils/Trimmer.h"

std::string Trimmer::trim(const std::string& str, const char* allToBeTrimmed)
{
    std::string result("");

    size_t first = str.find_first_not_of(allToBeTrimmed);

    if (first != std::string::npos) {
        size_t last = str.find_last_not_of(allToBeTrimmed);
        result = str.substr(first, (last-first+1));
    }

    return result;
}