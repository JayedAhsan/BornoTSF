// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once 

#include <locale>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Khipro {
 public:

  Khipro();

  std::string transliterate(const std::string& input);

};
