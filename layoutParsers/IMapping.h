// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <boost/regex/v5/regex.hpp>

class IMapping {
public:
    virtual ~IMapping() = default;
    virtual const std::vector<std::pair<boost::wregex, std::wstring>>& GetRules() const = 0;
};
