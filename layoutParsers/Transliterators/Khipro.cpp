// SPDX-License-Identifier: GPL-3.0-or-later

#include "Khipro.h"
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <shlobj.h>
#include "../khipro-port/mim_engine.h"

static std::string GetKhiproMimSpec()
{
    std::vector<std::wstring> candidatePaths;

    
    WCHAR appDataPath[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath)))
    {
        std::wstring base = appDataPath;
        candidatePaths.push_back(base + L"\\BornoTSF\\layouts\\bn-khipro.mim");
        candidatePaths.push_back(base + L"\\BornoTSF\\layouts\\khipro.mim");
    }

    
    WCHAR progDataPath[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, progDataPath)))
    {
        std::wstring base = progDataPath;
        candidatePaths.push_back(base + L"\\BornoTSF\\layouts\\bn-khipro.mim");
        candidatePaths.push_back(base + L"\\BornoTSF\\layouts\\khipro.mim");
    }

    for (const auto& path : candidatePaths)
    {
        DWORD attr = GetFileAttributesW(path.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::ifstream file(path.c_str(), std::ios::binary);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string spec = buffer.str();
                if (!spec.empty())
                {
                    return spec;
                }
            }
        }
    }

    return "";
}

static std::unique_ptr<khipro::KhiproEngine> g_khipro_engine;

Khipro::Khipro() {
    if (!g_khipro_engine) {
        std::string spec = GetKhiproMimSpec();
        if (!spec.empty()) {
            g_khipro_engine = std::unique_ptr<khipro::KhiproEngine>(new khipro::KhiproEngine(spec));
        }
    }
}

std::string Khipro::transliterate(const std::string& input) {
    if (!g_khipro_engine) {
        std::string spec = GetKhiproMimSpec();
        if (!spec.empty()) {
            g_khipro_engine = std::unique_ptr<khipro::KhiproEngine>(new khipro::KhiproEngine(spec));
        }
    }

    if (g_khipro_engine) {
        return g_khipro_engine->Convert(input);
    }
    return input;
}
