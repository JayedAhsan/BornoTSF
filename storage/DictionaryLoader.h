// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>
#include <windows.h>

class CDictionaryLoader
{
public:
    static CDictionaryLoader& GetInstance();

    bool LoadDictionary(const std::wstring& filePath = L"");
    bool GetAutoCorrect(const std::string& input, std::string& output);
    const std::unordered_map<std::string, std::string>& GetMap() const;
    bool IsLoaded() const { return _isLoaded; }

private:
    CDictionaryLoader();
    ~CDictionaryLoader();

    std::wstring GetDefaultDictionaryPath();
    bool ParseJsonMap(const std::string& jsonContent);

    std::unordered_map<std::string, std::string> _acMap;
    bool _isLoaded;
    CRITICAL_SECTION _cs;
};
