// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../IMapping.h"
#include <memory>

/**
 * @class National
 * @brief An implementation of the IMapping interface for the "Jatiyo" Bengali layout.
 *
 * This class holds the specific regex rules for the Jatiyo keyboard layout.
 */
class National : public IMapping {
public:
    National();
    const std::vector<std::pair<boost::wregex, std::wstring>>& GetRules() const override;

private:
    std::vector<std::pair<boost::wregex, std::wstring>> m_rules;
};
