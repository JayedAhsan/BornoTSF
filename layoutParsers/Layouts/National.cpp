// SPDX-License-Identifier: GPL-3.0-or-later

#include "National.h"
#include "../Common.h"
#include "../../core/Settings.h"
#include "../../core/Define.h"

National::National() {
}

const std::vector<std::pair<boost::wregex, std::wstring>>& National::GetRules() const {
    static DWORD lastState = 0xFFFFFFFF;
    static std::vector<std::pair<boost::wregex, std::wstring>> cached_rules;

    DWORD currentState = Settings::GetTypingStyle();
    if (!cached_rules.empty() && lastState == currentState) {
        return cached_rules;
    }

    lastState = currentState;
    bool isTraditional = (currentState == TYPING_STYLE_TRADITIONAL);

       cached_rules = {
        {boost::wregex(L"\""), L"`"},
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
        {boost::wregex(L"\\\\"), L"ৎ"},
        {boost::wregex(L"\\|"), L"ঃ"},
        {boost::wregex(L"g“"), L"~‌"},
        {boost::wregex(L"gg"), L"্‌"},
        {boost::wregex(L"vZ"), L"র‍্য"},
        {boost::wregex(L"g1"), L"৴"},
        {boost::wregex(L"g!"), L"ঀ"},
        {boost::wregex(L"g2"), L"৵"},
        {boost::wregex(L"g3"), L"৶"},
        {boost::wregex(L"g#"), L"ঽ"},
        {boost::wregex(L"g4"), L"৷"},
        {boost::wregex(L"g\\$"), L"৲"},
        {boost::wregex(L"g5"), L"৸"},
        {boost::wregex(L"g%"), L"ৄ"},
        {boost::wregex(L"g6"), L"ৠ"},
        {boost::wregex(L"g\\^"), L"ৠ"},
        {boost::wregex(L"g7"), L"৺"},
        {boost::wregex(L"g&"), L"ৼ"},
        {boost::wregex(L"g8"), L"ৡ"},
        {boost::wregex(L"g\\*"), L"ৣ"},
        {boost::wregex(L"g9"), L"ঌ"},
        {boost::wregex(L"g\\("), L"ৢ"},
        {boost::wregex(L"g0"), L"৹"},
        {boost::wregex(L"g\\)"), L"৽"},
        {boost::wregex(L"g-"), L"×"},
        {boost::wregex(L"g_"), L"÷"},
        {boost::wregex(L"gG"), L"॥"},
        {boost::wregex(L"g,"), L"ৰ"},
        {boost::wregex(L"g\\."), L"ৱ"},
        {boost::wregex(L"gf"), L"আ"},
        {boost::wregex(L"gs"), L"উ"},
        {boost::wregex(L"gx"), L"ো"},
        {boost::wregex(L"gX"), L"ঔ"},
        {boost::wregex(L"gc"), L"এ"},
        {boost::wregex(L"gC"), L"ঐ"},
        {boost::wregex(L"ga"), L"ঋ"},
        {boost::wregex(L"gS"), L"ঊ"},
        {boost::wregex(L"gd"), L"ই"},
        {boost::wregex(L"gD"), L"ঈ"},
        {boost::wregex(L"Ff"), L"আ"},
        {boost::wregex(L"F"), L"অ"},
        {boost::wregex(L"q"), L"ঙ"},
        {boost::wregex(L"Q"), L"ং"},
        {boost::wregex(L"w"), L"য"},
        {boost::wregex(L"W"), L"য়"},
        {boost::wregex(L"e"), L"ড"},
        {boost::wregex(L"E"), L"ঢ"},
        {boost::wregex(L"r"), L"প"},
        {boost::wregex(L"R"), L"ফ"},
        {boost::wregex(L"t"), L"ট"},
        {boost::wregex(L"T"), L"ঠ"},
        {boost::wregex(L"y"), L"চ"},
        {boost::wregex(L"Y"), L"ছ"},
        {boost::wregex(L"u"), L"জ"},
        {boost::wregex(L"U"), L"ঝ"},
        {boost::wregex(L"i"), L"হ"},
        {boost::wregex(L"I"), L"ঞ"},
        {boost::wregex(L"o"), L"গ"},
        {boost::wregex(L"O"), L"ঘ"},
        {boost::wregex(L"p"), L"ড়"},
        {boost::wregex(L"P"), L"ঢ়"},
        {boost::wregex(L"h"), L"ব"},
        {boost::wregex(L"H"), L"ভ"},
        {boost::wregex(L"j"), L"ক"},
        {boost::wregex(L"J"), L"খ"},
        {boost::wregex(L"k"), L"ত"},
        {boost::wregex(L"K"), L"থ"},
        {boost::wregex(L"l"), L"দ"},
        {boost::wregex(L"L"), L"ধ"},
        {boost::wregex(L"v"), L"র"},
        {boost::wregex(L"V"), L"ল"},
        {boost::wregex(L"b"), L"ন"},
        {boost::wregex(L"B"), L"ণ"},
        {boost::wregex(L"n"), L"স"},
        {boost::wregex(L"N"), L"ষ"},
        {boost::wregex(L"m"), L"ম"},
        {boost::wregex(L"M"), L"শ"},
        {boost::wregex(L"sz"), L"্রু"},
        {boost::wregex(L"Sz"), L"্রূ"},
        {boost::wregex(L"z"), L"্র"},
        {boost::wregex(L"SZ"), L"্যূ"},
        {boost::wregex(L"sZ"), L"্যু"},
        {boost::wregex(L"Z"), L"্য"},
        {boost::wregex(L"G"), L"।"},
        {boost::wregex(L"g"), L"্"},
        {boost::wregex(L"([dcC][ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])A"), L"র্$1"},
        {boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])A"), L"র্$1"},
        {boost::wregex(L"([ক-হড়ঢ়য়])A"), L"র্$1"},
        {boost::wregex(L"A"), L"র্"},
    };
    
    if (isTraditional) {
        cached_rules.push_back({boost::wregex(L"c([ক-হড়ঢ়য়])f"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"c([ক-হড়ঢ়য়])X"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"f"), L"া"});
        cached_rules.push_back({boost::wregex(L"ঁ([া])"), L"$1ঁ"});
        cached_rules.push_back({boost::wregex(L"c([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])"), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"c([ক-হড়ঢ়য়])"), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"C([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"C([ক-হড়ঢ়য়])"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"d([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])"), L"$1ি"});
        cached_rules.push_back({boost::wregex(L"d([ক-হড়ঢ়য়])"), L"$1ি"});
    } else {
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])cf"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])cf"), L"$1ো"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])cX"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])cX"), L"$1ৌ"});
        cached_rules.push_back({boost::wregex(L"f"), L"া"});
        cached_rules.push_back({boost::wregex(L"ঁ([া])"), L"$1ঁ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])c"), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])c"), L"$1ে"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])C"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])C"), L"$1ৈ"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়]্[ক-হড়ঢ়য়])d"), L"$1ি"});
        cached_rules.push_back({boost::wregex(L"([ক-হড়ঢ়য়])d"), L"$1ি"});
    }

    cached_rules.push_back({boost::wregex(L"a"), L"ৃ"});
    cached_rules.push_back({boost::wregex(L"s"), L"ু"});
    cached_rules.push_back({boost::wregex(L"S"), L"ূ"});
    cached_rules.push_back({boost::wregex(L"D"), L"ী"});
    cached_rules.push_back({boost::wregex(L"x"), L"ও"});
    cached_rules.push_back({boost::wregex(L"X"), L"ৗ"});
    
    cached_rules.push_back({boost::wregex(L"c"), L"ে"});
    cached_rules.push_back({boost::wregex(L"C"), L"ৈ"});
    cached_rules.push_back({boost::wregex(L"d"), L"ি"});
    
    cached_rules.push_back({boost::wregex(L"([ািীুূৃেৈোৌ])্র"), L"্র$1"});
    cached_rules.push_back({boost::wregex(L"([ািীুূৃেৈোৌ])্য"), L"্য$1"});
    return cached_rules;
}
