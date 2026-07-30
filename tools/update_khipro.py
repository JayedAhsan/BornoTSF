import sys
import os
import argparse

# Resolve paths relative to repository root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..'))
default_mim = os.path.join(project_root, 'layoutParsers', 'khipro-port', 'bn-khipro.mim')

parser = argparse.ArgumentParser(description="convert Khipro .mim specification file into C++ byte array in Khipro.cpp.")
parser.add_argument("mim_file", nargs="?", default=default_mim, help="Path to input .mim fil")
args = parser.parse_args()

mim_path = os.path.abspath(args.mim_file)
cpp_path = os.path.join(project_root, 'layoutParsers', 'Transliterators', 'Khipro.cpp')

if not os.path.exists(mim_path):
    print(f"Error: Could not find MIM file at '{mim_path}'.")
    sys.exit(1)

with open(mim_path, 'r', encoding='utf-8') as f:
    mim_content = f.read()

mim_bytes = mim_content.encode('utf-8')
hex_lines = []
for i in range(0, len(mim_bytes), 16):
    chunk = mim_bytes[i:i+16]
    hex_lines.append('    ' + ', '.join(f'0x{b:02x}' for b in chunk) + ',')
hex_array_str = '\n'.join(hex_lines)

new_cpp_content = f'''
#include "Khipro.h"
#include <memory>
#include "../khipro-port/mim_engine.h"


static const unsigned char khipro_mim_spec[] = {{
{hex_array_str}
    0x00
}};

static std::unique_ptr<khipro::KhiproEngine> g_khipro_engine;


Khipro::Khipro() {{
    if (!g_khipro_engine) {{
        g_khipro_engine = std::unique_ptr<khipro::KhiproEngine>(new khipro::KhiproEngine(reinterpret_cast<const char*>(khipro_mim_spec)));
    }}
}}


std::string Khipro::transliterate(const std::string& input) {{
    if (g_khipro_engine) {{
        return g_khipro_engine->Convert(input);
    }}
    return input;
}}
'''

with open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(new_cpp_content)

print(f"Updated '{cpp_path}' successfully using '{mim_path}'.")
