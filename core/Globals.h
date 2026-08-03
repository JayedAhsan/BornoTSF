// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include "Private.h"
#include "Define.h"
#include "BornoTSFBaseStructure.h"

void DllAddRef();
void DllRelease();

namespace Global {
//---------------------------------------------------------------------
// inline
//---------------------------------------------------------------------

inline void SafeRelease(_In_ IUnknown *punk)
{
    if (punk != nullptr)
    {
        punk->Release();
    }
}

//---------------------------------------------------------------------
// extern
//---------------------------------------------------------------------
extern HINSTANCE dllInstanceHandle;

extern LONG dllRefCount;

extern BOOL phoneticCorrection;
extern DWORD typingStyle;
extern DWORD currentLayout;

extern std::wstring enabledLayouts;



extern CRITICAL_SECTION CS;


extern const CLSID BornoTSFCLSID;
extern const CLSID BornoTSFGuidProfile;
extern const GUID GuidProfileAvroPhonetic;
extern const GUID GuidProfileKhipro;
extern const GUID GuidProfileJatiyo;
extern const GUID GuidProfileProbhat;
extern const CLSID BornoTSFGuidImeModePreserveKey;

LRESULT CALLBACK ThreadKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL CheckModifiers(UINT uModCurrent, UINT uMod);
BOOL UpdateModifiers(WPARAM wParam, LPARAM lParam);

extern USHORT ModifiersValue;
extern BOOL IsShiftKeyDownOnly;
extern BOOL IsControlKeyDownOnly;
extern BOOL IsAltKeyDownOnly;
extern UINT CandidateCommitKey; 
extern BOOL isImeEnabled; //toggle 8/1/26

extern const WCHAR FullWidthCharTable[];

extern const GUID BornoTSFGuidLangBarIMEMode;

extern const GUID BornoTSFGuidDisplayAttributeInput;
extern const GUID BornoTSFGuidDisplayAttributeConverted;

extern const WCHAR UnicodeByteOrderMark;

extern const WCHAR ImeModeDescription[];
extern const int ImeModeOnIcoIndex;
extern const int ImeModeOffIcoIndex;

extern const WCHAR LangbarImeModeDescription[];
}