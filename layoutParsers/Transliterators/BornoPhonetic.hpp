// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BORNO_PHONETIC_HPP
#define BORNO_PHONETIC_HPP

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <set>
#include "AutoCorrectMap.hpp"


namespace {
    const std::string HOSHONTO = "্";
    const std::string ZWJ = "\u200D";   
    const std::string ZWNJ = "\u200C";  

    inline bool IsKarSign(wchar_t c) {
        return (c >= 0x09BE && c <= 0x09C4) || (c >= 0x09C7 && c <= 0x09C8) || (c >= 0x09CB && c <= 0x09CC) || (c == 0x09D7);
    }

    inline wchar_t GetCorrespondingVowel(wchar_t sign) {
        switch (sign) {
            case 0x09BE: return 0x0986; 
            case 0x09BF: return 0x0987; 
            case 0x09C0: return 0x0988; 
            case 0x09C1: return 0x0989; 
            case 0x09C2: return 0x098A; 
            case 0x09C3: return 0x098B; 
            case 0x09C7: return 0x098F; 
            case 0x09C8: return 0x0990; 
            case 0x09CB: return 0x0993; 
            case 0x09CC: return 0x0994; 
            case 0x09D7: return 0x0994; 
            default: return sign;
        }
    }

    inline std::string RemoveConsecutiveKarSigns(const std::string& str) {
        std::string result = str;
        if (result.length() < 2) return result;
        for (size_t i = 1; i < result.length(); ++i) {
            if (IsKarSign(result[i - 1]) && IsKarSign(result[i])) {
                result[i] = GetCorrespondingVowel(result[i]);
            }
        }
        return result;
    }
}
// a bit messy, but works 20
class bornoPhonetic {
public:
    bornoPhonetic() : max_token_length(0) {
        init();
        initialize_global_keys();
    }

    std::string Convert(const std::string query) {
        return parse(query);
    }

    std::string parse(const std::string& input) {
        if (input.empty()) {
            return "";
        }

        std::string fixed;
        if (getAutoCorrect(input, fixed)) {
            return fixed;
        }

        size_t pos = fixed.find('\\');
        if (pos != std::string::npos) {
            size_t k = 0;
            while (pos + k < fixed.size() && fixed[pos + k] == '\\') {
                k++;
            }

            std::string left_text = fixed.substr(0, pos);
            std::string right_text = fixed.substr(pos + k);

            if (right_text.empty()) {
               
                switch (k) {
                    case 1:
                        return parseSegment(left_text) + "\\";
                    case 2:
                        return left_text;
                    case 3:
                        return left_text + "\\";
                    case 4:
                        return parseSegment(left_text) + "\\\\";
                    default:
                        if (k % 2 == 1) {
                            return left_text + std::string((k - 1) / 2, '\\');
                        } else {
                            return parseSegment(left_text) + std::string(k / 2, '\\');
                        }
                }
            } else {
               
                switch (k) {
                    case 1:
                        return parseSegment(left_text) + "\\" + parse(right_text);
                    case 2:
                        return left_text + parse(right_text);
                    case 3:
                        return left_text + "\\" + parse(right_text);
                    case 4:
                        return parseSegment(left_text) + "\\\\" + parse(right_text);
                    default:
                        if (k % 2 == 1) {
                            return left_text + std::string((k - 1) / 2, '\\') + parse(right_text);
                        } else {
                            return parseSegment(left_text) + std::string(k / 2, '\\') + parse(right_text);
                        }
                }
            }
        }

        return parseSegment(fixed);
    }

 
    std::string parseSegment(const std::string& input) {
        if (input.empty()) {
            return "";
        }

        std::string corrected_input = caseHandle(input);
        std::vector<std::string> tokens = tokenize(corrected_input);
        std::string output_str;
        output_str.reserve(corrected_input.length() * 3);

        enum PrevTokenType { START_OF_WORD, CONSONANT, VOWEL, SYMBOL, OTHER };
        PrevTokenType last_token_type = START_OF_WORD;

        for (size_t i = 0; i < tokens.size(); ++i) {
            const std::string& token = tokens[i];

            if (token == " ") {
                output_str += " ";
                last_token_type = START_OF_WORD;
                continue;
            }

 
            if (token == "w") {
                if (last_token_type == CONSONANT) {
                    output_str += HOSHONTO + phola_map.at("w");
                    last_token_type = CONSONANT;
                }
                else if (last_token_type == SYMBOL) {
                    output_str += phola_map.at("w");
                    last_token_type = CONSONANT;
                }
                else {
                    if (i + 1 < tokens.size() && tokens[i + 1] == "a") {
                        output_str += "ওয়া";
                        i++;
                    }
                    else if (i + 1 < tokens.size() && tokens[i + 1] == "e") {
                        output_str += "ওয়ে";
                        i++;
                    }
                    else {
                        output_str += "ও";
                    }
                    last_token_type = VOWEL;
                }
                continue;
            }

            
            if (token == "x") {
                if (last_token_type == START_OF_WORD) {
                    output_str += "এক্স";
                } else {
                    output_str += "ক্স";
                }
                last_token_type = CONSONANT;
                continue;
            }

            
            if (token == "rri") {
                bool next_is_backtick = (i + 1 < tokens.size() && tokens[i+1] == "`");
                if (next_is_backtick) {
                    output_str += "ৃ";
                } else if (last_token_type != CONSONANT || last_token_type == START_OF_WORD) {
                    output_str += "ঋ";
                } else {
                    output_str += "ৃ";
                }
                last_token_type = VOWEL;
                continue;
            }

            auto it_sym = symbol_map.find(token);
            if (it_sym != symbol_map.end()) {
                output_str += it_sym->second;
                last_token_type = SYMBOL;
                continue;
            }

            auto it_digit = digits_map.find(token);
            if (it_digit != digits_map.end()) {
                output_str += it_digit->second;
                last_token_type = OTHER;
                continue;
            }

            auto it_punc = punctuations_map.find(token);
            if (it_punc != punctuations_map.end()) {
                output_str += it_punc->second;
                last_token_type = OTHER;
                continue;
            }

            auto it_reph = reph_map.find(token);
            if (it_reph != reph_map.end()) {
                if (last_token_type == CONSONANT) {

                    output_str += HOSHONTO + consonants_map.at("r") + consonants_map.at("r");
                    last_token_type = CONSONANT;
                } else {
                    bool cond_reph = false;
                    if (i + 1 < tokens.size()) {
                        const std::string& next_token = tokens[i+1];
                        bool next_is_vowel = (initial_map.count(next_token) != 0);
                        if (!next_is_vowel && next_token != "r" && next_token != "R") {
                            cond_reph = true;
                        }
                    }
                    if (cond_reph) {
                        output_str += it_reph->second;
                        last_token_type = OTHER;
                    } else {
                        output_str += "রর";
                        last_token_type = CONSONANT;
                    }
                }
                continue;
            }

            auto it_diac = diacritics_map.find(token);
            if (it_diac != diacritics_map.end()) {
                output_str += it_diac->second;
                if (token == ",," || token == ",." || token == ",..") {
                    last_token_type = CONSONANT;
                } else {
                    last_token_type = VOWEL;
                }
                continue;
            }

            auto it_conj = conjuncts_map.find(token);
            if (it_conj != conjuncts_map.end()) {
                output_str += it_conj->second;
                last_token_type = CONSONANT;
                continue;
            }

            auto it_phola = phola_map.find(token);
            if (it_phola != phola_map.end()) {
                if (last_token_type == CONSONANT) {
                    if (token == "y" || token == "z" || token == "Z") {
                        bool prev_is_r = (i > 0 && tokens[i-1] == "r");
                        if (prev_is_r) {
                            bool prev2_is_special_consonant = false;
                            if (i >= 2) {
                                const std::string& prev2_token = tokens[i-2];
                                auto it_c = consonants_map.find(prev2_token);
                                auto it_j = conjuncts_map.find(prev2_token);
                                if ((it_c != consonants_map.end() || it_j != conjuncts_map.end()) &&
                                    prev2_token != "r" && prev2_token != "y" && prev2_token != "w" && prev2_token != "x") {
                                    prev2_is_special_consonant = true;
                                }
                            }
                            if (prev2_is_special_consonant) {
                                output_str += HOSHONTO + it_phola->second;
                            } else {
                                output_str += ZWJ + HOSHONTO + it_phola->second;
                            }
                        } else {
                            output_str += HOSHONTO + it_phola->second;
                        }
                    }
                    else if (token == "r") {
                        std::string ra_phola = HOSHONTO + it_phola->second;
                        if (output_str.length() >= ra_phola.length() &&
                            output_str.compare(output_str.length() - ra_phola.length(),
                                               ra_phola.length(), ra_phola) == 0) {
                            output_str += consonants_map.at("r");
                        } else {
                            output_str += HOSHONTO + it_phola->second;
                        }
                    }
                    else {
                        output_str += HOSHONTO + it_phola->second;
                    }
                }
                else {
                    if (token == "r") {
                        output_str += consonants_map.at("r");
                    }
                    else if (token == "R") {
                        output_str += consonants_map.at("R");
                    }
                    else if (token == "y" || token == "Y") {
                        output_str += consonants_map.at("y");
                    }
                    else if (token == "z" || token == "Z") {
                        output_str += consonants_map.at("z");
                    }
                }
                last_token_type = CONSONANT;
                continue;
            }

            auto it_cons = consonants_map.find(token);
            if (it_cons != consonants_map.end()) {
                output_str += it_cons->second;
                last_token_type = CONSONANT;
                continue;
            }

            auto it_init = initial_map.find(token);
            if (it_init != initial_map.end()) {
                if (last_token_type == CONSONANT) {
                    if (token == "o") {
                        
                    } else {
                        auto it = vowel_signs_map.find(token);
                        if (it != vowel_signs_map.end()) output_str += it->second;
                    }
                }
                else if (last_token_type == VOWEL) {
                    if (token == "o") {
                        output_str += "ও";
                    }
                    else {
                        auto it = after_vowel_map.find(token);
                        if (it != after_vowel_map.end()) output_str += it->second;
                    }
                }
                else {
                    output_str += it_init->second;
                }
                last_token_type = VOWEL;
                continue;
            }

            output_str += token;
            last_token_type = OTHER;
        }


        return RemoveConsecutiveKarSigns(output_str);
    }

private:
    std::unordered_map<std::string, std::string> initial_map;
    std::unordered_map<std::string, std::string> after_vowel_map;
    std::unordered_map<std::string, std::string> vowel_signs_map;
    std::unordered_map<std::string, std::string> consonants_map;
    std::unordered_map<std::string, std::string> conjuncts_map;
    std::unordered_map<std::string, std::string> diacritics_map;
    std::unordered_map<std::string, std::string> punctuations_map;
    std::unordered_map<std::string, std::string> phola_map;
    std::unordered_map<std::string, std::string> reph_map;
    std::unordered_map<std::string, std::string> digits_map;
    std::unordered_map<std::string, std::string> symbol_map;

    std::set<std::string, std::less<>> global_keys_set;
    size_t max_token_length;

    void init() {
        symbol_map = {
                {"ng", "ং"}, {"TH", "ৎ"}, {"X", "ৎ"}, {"t``", "ৎ"},
                {"xx", "ঁ"}, {"::", "ঃ"}
        };

        initial_map = {
                {"a", "আ"}, {"A", "আ"}, {"i", "ই"}, {"I", "ঈ"}, {"u", "উ"}, {"U", "ঊ"},
                {"e", "এ"}, {"E", "এ"}, {"o", "অ"}, {"O", "ও"}, {"OI", "ঐ"}, {"OU", "ঔ"}
        };

        after_vowel_map = {
                {"a", "আ"}, {"A", "আ"}, {"i", "ই"}, {"I", "ঈ"}, {"u", "উ"}, {"U", "ঊ"},
                {"e", "এ"}, {"E", "এ"}, {"o", "অ"}, {"O", "ও"}, {"OI", "ঐ"}, {"OU", "ঔ"}
        };

        vowel_signs_map = {
                {"a", "া"}, {"A", "া"}, {"i", "ি"}, {"I", "ী"}, {"u", "ু"}, {"U", "ূ"},
                {"e", "ে"}, {"E", "ে"}, {"o", ""},  {"O", "ো"}, {"OI", "ৈ"}, {"OU", "ৌ"}
        };

        consonants_map = {
                {"k", "ক"}, {"kh", "খ"}, {"g", "গ"}, {"G", "গ"}, {"gh", "ঘ"}, {"Gh", "ঘ"},
                {"Ng", "ঙ"}, {"NG", "ঞ"}, {"c", "চ"}, {"ch", "ছ"}, {"j", "জ"}, {"J", "জ"},
                {"jh", "ঝ"}, {"t", "ত"}, {"th", "থ"}, {"T", "ট"}, {"Th", "ঠ"}, {"d", "দ"},
                {"dh", "ধ"}, {"D", "ড"}, {"Dh", "ঢ"}, {"n", "ন"}, {"N", "ণ"}, {"p", "প"},
                {"ph", "ফ"}, {"f", "ফ"}, {"b", "ব"}, {"bh", "ভ"}, {"v", "ভ"}, {"m", "ম"},
                {"z", "য"}, {"y", "য়"}, {"Y", "য়"}, {"r", "র"}, {"R", "ড়"}, {"Rh", "ঢ়"},
                {"l", "ল"}, {"s", "স"}, {"S", "শ"}, {"sh", "শ"}, {"Sh", "ষ"}, {"SH", "ষ"}, {"h", "হ"}, {"H", "হ"},
                {"q", "ক"}, {"x", "ক্স"}
        };

        conjuncts_map = {
                {"kkhN", "ক্ষ্ণ"}, {"kShN", "ক্ষ্ণ"}, {"kkhm", "ক্ষ্ম"}, {"kShm", "ক্ষ্ম"},
                {"kxN", "ক্ষ্ণ"}, {"kxm", "ক্ষ্ম"}, {"kkh", "ক্ষ"}, {"kSh", "ক্ষ"},
                {"ksh", "ক্শ"}, {"kx", "ক্ষ"}, {"kk", "ক্ক"}, {"kT", "ক্ট"},
                {"kt", "ক্ত"}, {"km", "ক্ম"}, {"kl", "ক্ল"}, {"ks", "ক্স"},
                {"ghn", "ঘ্ন"}, {"Ghn", "ঘ্ন"}, {"gdh", "গ্ধ"}, {"Gdh", "গ্ধ"},
                {"gN", "গ্ণ"}, {"GN", "গ্ণ"}, {"gn", "গ্ন"}, {"Gn", "গ্ন"},
                {"gm", "গ্ম"}, {"Gm", "গ্ম"}, {"gl", "গ্ল"}, {"Gl", "গ্ল"},
                {"gg", "জ্ঞ"}, {"GG", "জ্ঞ"}, {"Gg", "জ্ঞ"}, {"gG", "জ্ঞ"},
                {"nch", "ঞ্ছ"}, {"njh", "ঞ্ঝ"}, {"ndh", "ন্ধ"}, {"nTh", "ন্ঠ"},
                {"nth", "ন্থ"}, {"nj", "ঞ্জ"}, {"nn", "ন্ন"}, {"nm", "ন্ম"},
                {"nd", "ন্দ"}, {"nT", "ন্ট"}, {"nD", "ন্ড"}, {"nt", "ন্ত"},
                {"ns", "ন্স"}, {"nc", "ঞ্চ"},
                {"NgkSh", "ঙ্ক্ষ"}, {"Ngkkh", "ঙ্ক্ষ"}, {"NGch", "ঞ্ছ"}, {"Nggh", "ঙ্ঘ"},
                {"Ngkh", "ঙ্খ"}, {"NGjh", "ঞ্ঝ"}, {"ngOI", "ঙ্গৈ"}, {"ngOU", "ঙ্গৌ"},
                {"Ngkx", "ঙ্ক্ষ"}, {"NGc", "ঞ্চ"}, {"ngh", "ঙ্ঘ"}, {"ngH", "ংহ"}, {"Ngk", "ঙ্ক"},
                {"Ngx", "ঙ্ক্ষ"}, {"Ngg", "ঙ্গ"}, {"Ngm", "ঙ্ম"}, {"NGj", "ঞ্জ"},
                {"NTh", "ণ্ঠ"}, {"nkh", "ঙ্খ"}, {"ngo", "ঙ্গ"}, {"nga", "ঙ্গা"},
                {"ngi", "ঙ্গি"}, {"ngI", "ঙ্গী"}, {"ngu", "ঙ্গু"}, {"ngU", "ঙ্গূ"},
                {"nge", "ঙ্গে"}, {"ngO", "ঙ্গো"}, {"NDh", "ণ্ঢ"}, {"nsh", "নশ"},
                {"Ngr", "ঙ্র"}, {"NGr", "ঞ্র"}, {"Ng", "ঙ"},
                {"NG", "ঞ"}, {"nk", "ঙ্ক"}, {"NN", "ণ্ণ"},
                {"Nn", "ণ্ণ"}, {"Nm", "ণ্ম"}, {"NT", "ণ্ট"}, {"ND", "ণ্ড"},
                {"cNG", "চ্ঞ"}, {"cch", "চ্ছ"}, {"cc", "চ্চ"},
                {"jjh", "জ্ঝ"}, {"jNG", "জ্ঞ"}, {"jj", "জ্জ"},
                {"tth", "ত্থ"}, {"tn", "ত্ন"}, {"tm", "ত্ম"}, {"tt", "ত্ত"},
                {"TT", "ট্ট"}, {"Tm", "ট্ম"},
                {"dhn", "ধ্ন"}, {"dhm", "ধ্ম"}, {"dgh", "দ্ঘ"}, {"ddh", "দ্ধ"},
                {"dbh", "দ্ভ"}, {"dv", "দ্ভ"}, {"dm", "দ্ম"}, {"dg", "দ্গ"},
                {"dd", "দ্দ"}, {"DD", "ড্ড"},
                {"phl", "ফ্ল"}, {"pT", "প্ট"}, {"pt", "প্ত"}, {"pn", "প্ন"},
                {"pp", "প্প"}, {"pl", "প্ল"}, {"ps", "প্স"}, {"fl", "ফ্ল"},
                {"bdh", "ব্ধ"}, {"bhl", "ভ্ল"}, {"bj", "ব্জ"}, {"bd", "ব্দ"},
                {"bb", "ব্ব"}, {"bl", "ব্ল"}, {"vl", "ভ্ল"},
                {"mth", "ম্থ"}, {"mph", "ম্ফ"}, {"mbh", "ম্ভ"}, {"mpl", "মপ্ল"},
                {"mn", "ম্ন"}, {"mp", "ম্প"}, {"mv", "ম্ভ"}, {"mm", "ম্ম"},
                {"ml", "ম্ল"}, {"mb", "ম্ব"}, {"mf", "ম্ফ"},
                {"Rg", "ড়্গ"},

                {"oZ", "অ্য"},
                {"lbh", "ল্ভ"}, {"ldh", "ল্ধ"}, {"lkh", "ল্খ"}, {"lgh", "ল্ঘ"},
                {"lph", "ল্ফ"}, {"lk", "ল্ক"}, {"lg", "ল্গ"}, {"lT", "ল্ট"},
                {"lD", "ল্ড"}, {"lp", "ল্প"}, {"lv", "ল্ভ"}, {"lm", "ল্ম"},
                {"ll", "ল্ল"}, {"lb", "ল্ব"},
                {"shch", "শ্ছ"}, {"Sch", "শ্ছ"}, {"skl", "স্ক্ল"}, {"skh", "স্খ"},
                {"sth", "স্থ"}, {"sph", "স্ফ"}, {"shc", "শ্চ"}, {"sht", "শ্ত"},
                {"shn", "শ্ন"}, {"shm", "শ্ম"}, {"spl", "স্প্ল"}, {"shl", "শ্ল"},
                {"sk", "স্ক"}, {"Sc", "শ্চ"}, {"sT", "স্ট"}, {"st", "স্ত"},
                {"sn", "স্ন"}, {"sp", "স্প"}, {"sf", "স্ফ"}, {"sm", "স্ম"},
                {"sl", "স্ল"},
                {"ShTh", "ষ্ঠ"}, {"Shph", "ষ্ফ"}, {"Shk", "ষ্ক"}, {"ShT", "ষ্ট"},
                {"ShN", "ষ্ণ"}, {"Shp", "ষ্প"}, {"Shf", "ষ্ফ"}, {"Shm", "ষ্ম"},
                {"SHTh", "ষ্ঠ"}, {"SHph", "ষ্ফ"}, {"SHk", "ষ্ক"}, {"SHT", "ষ্ট"},
                {"SHN", "ষ্ণ"}, {"SHp", "ষ্প"}, {"SHf", "ষ্ফ"}, {"SHm", "ষ্ম"},
                {"Sn", "শ্ন"}, {"Sm", "শ্ম"}, {"Sl", "শ্ল"},
                {"hN", "হ্ণ"}, {"hn", "হ্ন"}, {"hm", "হ্ম"}, {"hl", "হ্ল"}
        };

        diacritics_map = {
                {"^`", "^"}, {":`", ":"},
                {",,", HOSHONTO}, {",.", HOSHONTO + ZWNJ}, {",..", HOSHONTO + ZWJ},
                {"a`", "া"}, {"i`", "ি"}, {"I`", "ী"}, {"u`", "ু"}, {"U`", "ূ"},
                {"e`", "ে"}, {"o`", ""}, {"O`", "ো"}, {"OI`", "ৈ"}, {"OU`", "ৌ"},
                {"rri`", "ৃ"}, {"./", "়"}
        };

        punctuations_map = {
                {"`", ""}, {"``", "`"}, {".", "।"}, {"..", "।।"}, {"...", "..."},
                {"$$", "$"}, {",", ","}, {"$", "৳"}, {"$%", "₹"}
        };

        phola_map = {
                {"y", "য"}, {"z", "য"}, {"Z", "য"}, {"r", "র"}, {"w", "ব"}
        };

        reph_map = {
                {"rr", "র্"}
        };

        digits_map = {
                {".1", ".১"}, {".2", ".২"}, {".3", ".৩"}, {".4", ".৪"}, {".5", ".৫"},
                {".6", ".৬"}, {".7", ".৭"}, {".8", ".৮"}, {".9", ".৯"}, {".0", ".০"},
                {"1", "১"}, {"2", "২"}, {"3", "৩"}, {"4", "৪"}, {"5", "৫"},
                {"6", "৬"}, {"7", "৭"}, {"8", "৮"}, {"9", "৯"}, {"0", "০"}
        };
    }

    void initialize_global_keys() {
        auto populate_mapping = [&](const auto& map_to_add) {
            for (const auto& pair : map_to_add) {
                global_keys_set.insert(pair.first);
                if (pair.first.length() > max_token_length) {
                    max_token_length = pair.first.length();
                }
            }
        };

        populate_mapping(symbol_map);
        populate_mapping(initial_map);
        populate_mapping(after_vowel_map);
        populate_mapping(vowel_signs_map);
        populate_mapping(consonants_map);
        populate_mapping(conjuncts_map);
        populate_mapping(diacritics_map);
        populate_mapping(punctuations_map);
        populate_mapping(phola_map);
        populate_mapping(reph_map);
        populate_mapping(digits_map);
        global_keys_set.insert("rri");
        if (3 > max_token_length) max_token_length = 3;
    }

    std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        if (text.empty()) {
            return tokens;
        }

        std::string_view sv(text);
        size_t i = 0;
        while (i < sv.length()) {
            bool matched = false;
            for (size_t l = max_token_length; l > 0; --l) {
                if (i + l <= sv.length()) {
                    std::string_view segment = sv.substr(i, l);
                    if (global_keys_set.count(segment)) {
                        
                        if (l > 2 && segment.back() == 'r' &&
                            i + l + 2 <= sv.length() &&
                            sv[i + l] == 'r' && sv[i + l + 1] == 'i') {
                            std::string_view shorter = sv.substr(i, l - 1);
                            if (global_keys_set.count(shorter)) {
                                tokens.push_back(std::string(shorter));
                                i += l - 1;
                                matched = true;
                                break;
                            }
                        }
                        
                        if (l > 1 && segment.back() == 'T' &&
                            i + l < sv.length() &&
                            sv[i + l] == 'H') {
                            std::string_view shorter = sv.substr(i, l - 1);
                            if (global_keys_set.count(shorter)) {
                                tokens.push_back(std::string(shorter));
                                i += l - 1;
                                matched = true;
                                break;
                            }
                        }
                        tokens.push_back(std::string(segment));
                        i += l;
                        matched = true;
                        break;
                    }
                }
            }
            if (!matched) {
                tokens.push_back(std::string(sv.substr(i, 1)));
                i += 1;
            }
        }
        return tokens;
    }

    std::string caseHandle(const std::string& in) const {
        std::string s = in;
        for (char& c : s) {
            if (!((c == 'a') || (c == 'A') || (c == 'o') || (c == 'O') ||
                  (c == 'i') || (c == 'I') || (c == 'u') || (c == 'U') ||
                  (c == 'd') || (c == 'D') || (c == 'g') || (c == 'G') ||
                  (c == 'j') || (c == 'J') || (c == 'n') || (c == 'N') ||
                  (c == 'r') || (c == 'R') || (c == 's') || (c == 'S') ||
                  (c == 't') || (c == 'T') || (c == 'y') || (c == 'Y') ||
                  (c == 'z') || (c == 'Z') || (c == 'h') || (c == 'H')|| (c == 'x') || (c == 'X'))) {
                if (c >= 'A' && c <= 'Z') {
                    c += 32;
                }
            }
        }
        return s;
    }
};

#endif
