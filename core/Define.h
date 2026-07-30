// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "resource.h"
#include <tchar.h>
#include <strsafe.h>

#define TEXTSERVICE_MODEL        L"Apartment"
#define TEXTSERVICE_LANGID       MAKELANGID(LANG_BANGLA, SUBLANG_BANGLA_BANGLADESH)
#define TEXTSERVICE_ICON_INDEX   -IDIS_BornoTSF

#define IME_MODE_ON_ICON_INDEX      IDI_IME_MODE_ON
#define IME_MODE_OFF_ICON_INDEX     IDIS_BornoTSF

#define BORNO_PHONETIC 1
#define BORNO_KHIPRO 2
#define BORNO_AVRO 3
#define LAYOUT_JATIYO 5
#define LAYOUT_PROBHAT 6

#define TYPING_STYLE_NORMAL 0
#define TYPING_STYLE_TRADITIONAL 1


//---------------------------------------------------------------------
// defined modifier
//---------------------------------------------------------------------
#define _TF_MOD_ON_KEYUP_SHIFT_ONLY    (0x00010000 | TF_MOD_ON_KEYUP)
#define _TF_MOD_ON_KEYUP_CONTROL_ONLY  (0x00020000 | TF_MOD_ON_KEYUP)
#define _TF_MOD_ON_KEYUP_ALT_ONLY      (0x00040000 | TF_MOD_ON_KEYUP)

//---------------------------------------------------------------------
// string length of CLSID
//---------------------------------------------------------------------
#define CLSID_STRLEN    (38)  // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")

// Logging Helpers
inline void Log(const TCHAR* format, ...) {
#ifdef _DEBUG
  TCHAR buffer[512];
  va_list args;
  va_start(args, format);
  StringCchVPrintf(buffer, ARRAYSIZE(buffer), format, args);
  va_end(args);
  OutputDebugString(buffer);
  OutputDebugString(_T("\n"));
#else
  UNREFERENCED_PARAMETER(format);
#endif
}

inline void LogA(const CHAR* format, ...) {
#ifdef _DEBUG
  CHAR buffer[256];
  va_list args;
  va_start(args, format);
  StringCchVPrintfA(buffer, ARRAYSIZE(buffer), format, args);
  va_end(args);
  OutputDebugStringA(buffer);
  OutputDebugStringA("\n");
#else
  UNREFERENCED_PARAMETER(format);
#endif
}