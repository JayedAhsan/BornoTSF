import json
import os
import sys
import argparse

def escape_cpp_string(s: str) -> str:
    res = []
    for char in s:
        if char == '\\':
            res.append('\\\\')
        elif char == '"':
            res.append('\\"')
        elif char == '\n':
            res.append('\\n')
        elif char == '\r':
            res.append('\\r')
        elif char == '\t':
            res.append('\\t')
        else:
            res.append(char)
    return '"' + ''.join(res) + '"'

def convert_ac(json_path: str, output_path: str):
    if not os.path.exists(json_path):
        print(f"Error: Could not find JSON file at '{json_path}'.")
        sys.exit(1)

    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    entries = []
    for k, v in data.items():
        escaped_key = escape_cpp_string(k)
        escaped_val = escape_cpp_string(v)
        entries.append(f'    {{{escaped_key}, {escaped_val}}}')

    map_content = ',\n'.join(entries)

    hpp_content = f'''#ifndef AUTO_CORRECT_MAP_HPP
#define AUTO_CORRECT_MAP_HPP

#include <string>
#include <unordered_map>
#include "../../core/Settings.h"

struct ACEntry {{
    const char* key;
    const char* val;
}};

static const ACEntry g_acEntries[] = {{
{map_content}
}};

inline const std::unordered_map<std::string, std::string>& GetAutoCorrectMap() {{
    static std::unordered_map<std::string, std::string> acMap;
    static bool initialized = false;
    if (!initialized) {{
        acMap.reserve(sizeof(g_acEntries) / sizeof(g_acEntries[0]));
        for (const auto& entry : g_acEntries) {{
            acMap[entry.key] = entry.val;
        }}
        initialized = true;
    }}
    return acMap;
}}

inline bool getAutoCorrect(const std::string& input, std::string& output) {{
    if (!Settings::GetPhoneticCorrection()) {{
        output = input;
        return false;
    }}
    const auto& map = GetAutoCorrectMap();
    auto it = map.find(input);
    if (it != map.end()) {{
        output = it->second;
        return (it->first == it->second);
    }}
    output = input;
    return false;
}}

inline std::string fixInput(const std::string& input) {{
    if (!Settings::GetPhoneticCorrection()) {{
        return input;
    }}
    const auto& map = GetAutoCorrectMap();
    auto it = map.find(input);
    if (it != map.end()) {{
        return it->second;
    }}
    return input;
}}

#endif // AUTO_CORRECT_MAP_HPP
'''

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(hpp_content)

    print(f"Successfully converted '{json_path}' ({len(data)} entries) to '{output_path}'.")

if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))

    default_json = os.path.join(project_root, 'assets', 'entries', 'ac.json')
    default_out = os.path.join(project_root, 'layoutParsers', 'Transliterators', 'AutoCorrectMap.hpp')

    parser = argparse.ArgumentParser(description="Convert ac.json to C++ unordered_map header file.")
    parser.add_argument("json_file", nargs="?", default=default_json, help="Path to ac.json")
    parser.add_argument("out_file", nargs="?", default=default_out, help="Path to output header file")
    args = parser.parse_args()

    convert_ac(os.path.abspath(args.json_file), os.path.abspath(args.out_file))
