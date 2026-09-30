#include "Helper.h"
#include <cctype>
#include <string>

std::string Helper::toLower(std::string_view sv)
{
    std::string out { sv };
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return out;
}
