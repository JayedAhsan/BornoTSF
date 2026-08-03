// SPDX-License-Identifier: GPL-3.0-or-later

#include "Probhat.h"
#include "../Common.h"
#include "../../core/Settings.h"
#include "../../core/Define.h"

Probhat::Probhat() {
}

const std::vector<std::pair<boost::wregex, std::wstring>>& Probhat::GetRules() const {
    static DWORD lastState = 0xFFFFFFFF;
    static std::vector<std::pair<boost::wregex, std::wstring>> cached_rules;

    DWORD currentState = Settings::GetTypingStyle();
    if (!cached_rules.empty() && lastState == currentState) {
        return cached_rules;
    }

    lastState = currentState;
    bool isTraditional = (currentState == TYPING_STYLE_TRADITIONAL);

    cached_rules = {
        {boost::wregex(L"\""), L"”"},
        {boost::wregex(L"~"), L"“"},
        {boost::wregex(L"\\$"), L"৳"},
        {boost::wregex(L"&"), L"ঁ"},
        {boost::wregex(L"1"), L"১"},
        {boost::wregex(L"2"), L"২"},
        {boost::wregex(L"3"), L"৩"},
        {boost::wregex(L"4"), L"৪"},
        {boost::wregex(L"5"), L"৫"},
        {boost::wregex(L"6"), L"৬"},
        {boost::wregex(L"7"), L"৭"},
        {boost::wregex(L"8"), L"৮"},
        {boost::wregex(L"9"), L"৯"},
        {boost::wregex(L"0"), L"০"},

        {boost::wregex(L"/a"), L"আ"},
        {boost::wregex(L"/u"), L"উ"},
        {boost::wregex(L"/o"), L"ও"},
        {boost::wregex(L"/\\}"), L"ঔ"},
        {boost::wregex(L"\/\\["), L"এ"},
        {boost::wregex(L"\/\\{"), L"ঐ"},
        {boost::wregex(L"/,"), L"ঋ"},
        {boost::wregex(L"/w"), L"ঊ"},
        {boost::wregex(L"/e"), L"ঈ"},
        {boost::wregex(L"/i"), L"ই"},

        {boost::wregex(L"Aa"), L"আ"},
        {boost::wregex(L"A"), L"অ"},
        {boost::wregex(L"q"), L"দ"},
        {boost::wregex(L"Q"), L"ধ"},
        {boost::wregex(L"w"), L"ূ"},
        {boost::wregex(L"W"), L"ঊ"},
        {boost::wregex(L"e"), L"ী"},
        {boost::wregex(L"E"), L"ঈ"},
        {boost::wregex(L"r"), L"র"},
        {boost::wregex(L"R"), L"ড়"},
        {boost::wregex(L"t"), L"ট"},
        {boost::wregex(L"T"), L"ঠ"},
        {boost::wregex(L"y"), L"এ"},
        {boost::wregex(L"Y"), L"ঐ"},
        {boost::wregex(L"u"), L"ু"},
        {boost::wregex(L"U"), L"উ"},

        {boost::wregex(L"I"), L"ই"},
        {boost::wregex(L"o"), L"ও"},
        {boost::wregex(L"O"), L"ঔ"},
        {boost::wregex(L"p"), L"প"},
        {boost::wregex(L"P"), L"ফ"},
        {boost::wregex(L"h"), L"হ"},
        {boost::wregex(L"H"), L"ঃ"},
        {boost::wregex(L"j"), L"জ"},
        {boost::wregex(L"J"), L"ঝ"},
        {boost::wregex(L"k"), L"ক"},
        {boost::wregex(L"K"), L"খ"},
        {boost::wregex(L"l"), L"ল"},
        {boost::wregex(L"L"), L"ং"},
        {boost::wregex(L"v"), L"আ"},
        {boost::wregex(L"V"), L"ঋ"},
        {boost::wregex(L"b"), L"ব"},
        {boost::wregex(L"B"), L"ভ"},
        {boost::wregex(L"n"), L"ন"},
        {boost::wregex(L"N"), L"ণ"},
        {boost::wregex(L"m"), L"ম"},
        {boost::wregex(L"M"), L"ঙ"},

        {boost::wregex(L"z"), L"য়"},
        {boost::wregex(L"Z"), L"য"},
        {boost::wregex(L"G"), L"ঘ"},
        {boost::wregex(L"g"), L"গ"},
        {boost::wregex(L"/"), L"্"},
        {boost::wregex(L"([i\\[\\{][ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\|"), L"র্$1"},
        {boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\|"), L"র্$1"},
        {boost::wregex(L"([ক-হড়ঢ়য়])\\|"), L"র্$1"},
        {boost::wregex(L"\\|"), L"র্"},
        {boost::wregex(L"A"), L"অ"},
        {boost::wregex(L"f"), L"ত"},
    };

    if (isTraditional) {
 
        cached_rules.push_back({boost::wregex(L"\\[([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])a"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"\\[([ক-হড়ঢ়য়])a"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"\\[([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\}"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"\\[([ক-হড়ঢ়য়])\\}"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"\\[([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])"), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"\\[([ক-হড়ঢ়য়])"), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"\\{([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"\\{([ক-হড়ঢ়য়])"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"i([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])"), L"$1ি"});
        cached_rules.push_back({boost::wregex(L"i([ক-হড়ঢ়য়])"), L"$1ি"});
    } else {
        
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\[a"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])\\[a"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\[\\}"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])\\[\\}"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\["), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])\\["), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])\\{"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])\\{"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])i"), L"$1ি"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])i"), L"$1ি"});
    }

    cached_rules.push_back({boost::wregex(L"a"), L"া"});
    cached_rules.push_back({boost::wregex(L"s"), L"স"});
    cached_rules.push_back({boost::wregex(L"S"), L"ষ"});
    cached_rules.push_back({boost::wregex(L"d"), L"ঢ"});
    cached_rules.push_back({boost::wregex(L"D"), L"ী"});
    cached_rules.push_back({boost::wregex(L"x"), L"শ"});
    cached_rules.push_back({boost::wregex(L"X"), L"ঢ়"});

    cached_rules.push_back({boost::wregex(L"\\]"), L"ো"});
    cached_rules.push_back({boost::wregex(L"\\}"), L"ৌ"});
    cached_rules.push_back({boost::wregex(L"c"), L"চ"});
    cached_rules.push_back({boost::wregex(L"C"), L"ছ"});
    cached_rules.push_back({boost::wregex(L"\\["), L"ে"});
    cached_rules.push_back({boost::wregex(L"\\{"), L"ৈ"});
    cached_rules.push_back({boost::wregex(L"i"), L"ি"});
    cached_rules.push_back({boost::wregex(L"([ািীুূৃেৈোৌ])্র"), L"্র$1"});
    cached_rules.push_back({boost::wregex(L"([ািীুূৃেৈোৌ])্য"), L"্য$1"});

    return cached_rules;
}
