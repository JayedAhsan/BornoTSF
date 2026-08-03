#ifndef AUTO_CORRECT_MAP_HPP
#define AUTO_CORRECT_MAP_HPP

#include <string>
#include "../../core/Settings.h"
#include "../../storage/DictionaryLoader.h"
// replaced with Dictionary Loader
inline bool getAutoCorrect(const std::string& input, std::string& output) {
    if (!Settings::GetPhoneticCorrection()) {
        output = input;
        return false;
    }
    if (CDictionaryLoader::GetInstance().GetAutoCorrect(input, output)) {
        return (input == output);
    }
    output = input;
    return false;
}

inline std::string fixInput(const std::string& input) {
    if (!Settings::GetPhoneticCorrection()) {
        return input;
    }
    std::string output;
    if (CDictionaryLoader::GetInstance().GetAutoCorrect(input, output)) {
        return output;
    }
    return input;
}

#endif // AUTO_CORRECT_MAP_HPP
