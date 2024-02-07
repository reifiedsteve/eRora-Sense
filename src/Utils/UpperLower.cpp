#include "Utils/UpperLower.h"

std::string UpperLower::toUpper(const std::string& str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char ch){
            return std::toupper(ch);
        }
    );
    return result;
}

std::string UpperLower::toLower(const std::string& str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char ch){
            return std::tolower(ch);
        }
    );
    return result;
}