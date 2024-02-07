#pragma once

#include <string>

class Trimmer
{
public:

    static std::string trim(const std::string& str, const char* allToBeTrimmed = " \t\n\r");
};