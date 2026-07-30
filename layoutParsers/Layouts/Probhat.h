// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../IMapping.h"

/**
 * @class Probhat
 * @brief An implementation of the IMapping interface for the "Probhat" Bengali layout.
 *
 * This class holds the specific regex rules for the Probhat keyboard layout.
 */
class Probhat : public IMapping {
public:
    Probhat();
    const std::vector<std::pair<boost::wregex, std::wstring>>& GetRules() const override;
};
