#include "DictionaryLoader.h"
#include "../core/Globals.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <shlobj.h>

CDictionaryLoader& CDictionaryLoader::GetInstance()
{
    static CDictionaryLoader instance;
    return instance;
}

CDictionaryLoader::CDictionaryLoader() : _isLoaded(false)
{
    InitializeCriticalSection(&_cs);
}

CDictionaryLoader::~CDictionaryLoader()
{
    DeleteCriticalSection(&_cs);
}

std::wstring CDictionaryLoader::GetDefaultDictionaryPath()
{
    WCHAR appDataPath[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath)))
    {
        std::wstring userDict = std::wstring(appDataPath) + L"\\BornoTSF\\dictionaries\\ac.json";
        DWORD dwAttrib = GetFileAttributesW(userDict.c_str());
        if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            return userDict;
        }
    }

    WCHAR progDataPath[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, progDataPath)))
    {
        std::wstring sysDict = std::wstring(progDataPath) + L"\\BornoTSF\\dictionaries\\ac.json";
        DWORD dwAttrib = GetFileAttributesW(sysDict.c_str());
        if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            return sysDict;
        }
    }
    return L"";
}

static std::string ParseJsonString(const std::string& str, size_t& pos)
{

 //kinda hardcoded
    std::string result;
    if (pos >= str.length() || str[pos] != '"') return result;
    pos++; 

    while (pos < str.length())
    {
        char ch = str[pos++];
        if (ch == '"')
        {
            return result; // end
        }
        else if (ch == '\\' && pos < str.length())
        {
            char nextChar = str[pos++];
            switch (nextChar)
            {
            case '"':  result += '"'; break;
            case '\\': result += '\\'; break;
            case '/':  result += '/'; break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            case 'u':
                if (pos + 4 <= str.length())
                {
                    std::string hexStr = str.substr(pos, 4);
                    pos += 4;
                    unsigned int hexVal = 0;
                    std::stringstream ss;
                    ss << std::hex << hexStr;
                    ss >> hexVal;
                    if (hexVal < 0x80)
                    {
                        result += (char)hexVal;
                    }
                    else if (hexVal < 0x800)
                    {
                        result += (char)(0xC0 | (hexVal >> 6));
                        result += (char)(0x80 | (hexVal & 0x3F));
                    }
                    else
                    {
                        result += (char)(0xE0 | (hexVal >> 12));
                        result += (char)(0x80 | ((hexVal >> 6) & 0x3F));
                        result += (char)(0x80 | (hexVal & 0x3F));
                    }
                }
                break;
            default:
                result += nextChar;
                break;
            }
        }
        else
        {
            result += ch;
        }
    }
    return result;
}

bool CDictionaryLoader::ParseJsonMap(const std::string& jsonContent)
{
    std::unordered_map<std::string, std::string> tempMap;
    size_t pos = 0;
    size_t len = jsonContent.length();

    
    while (pos < len && jsonContent[pos] != '{') pos++;
    if (pos >= len) return false;
    pos++;

    while (pos < len)
    {
  
        while (pos < len && (jsonContent[pos] == ' ' || jsonContent[pos] == '\t' ||
                             jsonContent[pos] == '\r' || jsonContent[pos] == '\n' ||
                             jsonContent[pos] == ','))
        {
            pos++;
        }

        if (pos >= len || jsonContent[pos] == '}') break;

        if (jsonContent[pos] == '"')
        {
            std::string key = ParseJsonString(jsonContent, pos);

            // handled with the help of AI
            while (pos < len && (jsonContent[pos] == ' ' || jsonContent[pos] == '\t' ||
                                 jsonContent[pos] == '\r' || jsonContent[pos] == '\n'))
            {
                pos++;
            }
            if (pos < len && jsonContent[pos] == ':') pos++;
            while (pos < len && (jsonContent[pos] == ' ' || jsonContent[pos] == '\t' ||
                                 jsonContent[pos] == '\r' || jsonContent[pos] == '\n'))
            {
                pos++;
            }

            if (pos < len && jsonContent[pos] == '"')
            {
                std::string val = ParseJsonString(jsonContent, pos);
                tempMap[key] = val;
            }
        }
        else
        {
            pos++;
        }
    }

    if (!tempMap.empty())
    {
        EnterCriticalSection(&_cs);
        _acMap = std::move(tempMap);
        _isLoaded = true;
        LeaveCriticalSection(&_cs);
        return true;
    }

    return false;
}

bool CDictionaryLoader::LoadDictionary(const std::wstring& filePath)
{
    std::wstring targetPath = filePath;
    if (targetPath.empty())
    {
        targetPath = GetDefaultDictionaryPath();
    }

    if (targetPath.empty())
    {
        return false;
    }

    std::ifstream inFile(targetPath, std::ios::binary);
    if (!inFile.is_open())
    {
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)),
                         std::istreambuf_iterator<char>());
    inFile.close();

    return ParseJsonMap(content);
}

bool CDictionaryLoader::GetAutoCorrect(const std::string& input, std::string& output)
{
    EnterCriticalSection(&_cs);
    if (!_isLoaded)
    {
        LeaveCriticalSection(&_cs);
        if (!LoadDictionary())
        {
            return false;
        }
        EnterCriticalSection(&_cs);
    }

    auto it = _acMap.find(input);
    if (it != _acMap.end())
    {
        output = it->second;
        LeaveCriticalSection(&_cs);
        return true;
    }

    LeaveCriticalSection(&_cs);
    return false;
}

const std::unordered_map<std::string, std::string>& CDictionaryLoader::GetMap() const
{
    return _acMap;
}
