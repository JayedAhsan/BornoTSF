// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include <vector>
#include <sstream>
#include "Globals.h"

namespace Settings {

inline std::wstring GetEnabledLayouts() { return Global::enabledLayouts; }
inline void SetEnabledLayouts(const std::wstring& layouts) { Global::enabledLayouts = layouts; }

inline BOOL IsLayoutEnabled(DWORD layoutId) {
  if (Global::enabledLayouts.empty()) return (layoutId == BORNO_PHONETIC);
  std::wstring idStr = std::to_wstring(layoutId);
  std::wstring search = L"," + Global::enabledLayouts + L",";
  std::wstring target = L"," + idStr + L",";
  return search.find(target) != std::wstring::npos;
}

inline void SetLayoutEnabled(DWORD layoutId, BOOL enable) {
  std::wstring name = std::to_wstring(layoutId);
  std::vector<std::wstring> items;
  std::wstringstream ss(Global::enabledLayouts);
  std::wstring item;
  bool found = false;

  while (std::getline(ss, item, L',')) {
    if (item.empty()) continue;
    if (item == name) {
      found = true;
      if (enable) {
        items.push_back(item);
      }
    } else {
      items.push_back(item);
    }
  }
  if (enable && !found) {
    items.push_back(name);
  }

  std::wstring result;
  for (size_t i = 0; i < items.size(); i++) {
    if (i > 0) result += L",";
    result += items[i];
  }
  if (result.empty()) {
    result = std::to_wstring(BORNO_PHONETIC);
  }
  Global::enabledLayouts = result;
}


inline BOOL GetPhoneticCorrection() { return Global::phoneticCorrection; }
inline DWORD GetTypingStyle() { return Global::typingStyle; }
inline DWORD GetCurrentLayout() { return Global::currentLayout; }





inline void SetPhoneticCorrection(BOOL enable) {
  Global::phoneticCorrection = enable;
}

inline void SetTypingStyle(DWORD style) {
  Global::typingStyle = style;
}

inline void SetCurrentLayout(DWORD layout) {
  Global::currentLayout = layout;
}


}  // namespace Settings