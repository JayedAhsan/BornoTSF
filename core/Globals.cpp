// SPDX-License-Identifier: GPL-3.0-or-later

#include "Globals.h"
#include "Private.h"
#include "resource.h"
#include "Define.h"
#include "BornoTSFBaseStructure.h"

namespace Global {
HINSTANCE dllInstanceHandle;

LONG dllRefCount = -1;

BOOL phoneticCorrection = TRUE;

DWORD typingStyle = TYPING_STYLE_NORMAL;
DWORD currentLayout = 1;

std::wstring enabledLayouts = L"1,2,3,5,6";




CRITICAL_SECTION CS;


//---------------------------------------------------------------------
// BornoTSF CLSID {E81A2B3C-4F5D-6E7A-8B9C-0D1E2F3A4B5C}
//---------------------------------------------------------------------
extern const CLSID BornoTSFCLSID = {
    0xe81a2b3c,
    0x4f5d,
    0x6e7a,
    {0x8b, 0x9c, 0x0d, 0x1e, 0x2f, 0x3a, 0x4b, 0x5c}};

//---------------------------------------------------------------------
// Profile GUIDs
//---------------------------------------------------------------------
extern const GUID BornoTSFGuidProfile = {
    0xa71f9b3c, 0x4e28, 0x49f1, {0x8b, 0x56, 0x1c, 0x7d, 0x9a, 0x2e, 0x3f, 0x40}};

extern const GUID GuidProfileAvroPhonetic = {
    0xb82e0c4d, 0x5f39, 0x4a02, {0x9c, 0x67, 0x2d, 0x8e, 0x0b, 0x3f, 0x4a, 0x51}};

extern const GUID GuidProfileKhipro = {
    0xc93f1d5e, 0x6a40, 0x4b13, {0xad, 0x78, 0x3e, 0x9f, 0x1c, 0x4a, 0x5b, 0x62}};

extern const GUID GuidProfileJatiyo = {
    0xda402e6f, 0x7b51, 0x4c24, {0xbe, 0x89, 0x4f, 0xa0, 0x2d, 0x5b, 0x6c, 0x73}};

extern const GUID GuidProfileProbhat = {
    0xeb513f70, 0x8c62, 0x4d35, {0xcf, 0x90, 0x5a, 0xb1, 0x3e, 0x6c, 0x7d, 0x84}};

//---------------------------------------------------------------------
// PreserveKey GUID {C384B692-7E1A-4835-9271-D652B18E9403}
//---------------------------------------------------------------------
extern const GUID BornoTSFGuidImeModePreserveKey = {
    0xc384b692,
    0x7e1a,
    0x4835,
    {0x92, 0x71, 0xd6, 0x52, 0xb1, 0x8e, 0x94, 0x03}};

//---------------------------------------------------------------------
// LanguageBar GUID {F92B3C4D-5A6B-7C8D-9E0F-1A2B3C4D5E6F}
//---------------------------------------------------------------------
extern const GUID BornoTSFGuidLangBarIMEMode = {
    0xf92b3c4d,
    0x5a6b,
    0x7c8d,
    {0x9e, 0x0f, 0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f}};

//---------------------------------------------------------------------
// Display Attributes GUIDs
//---------------------------------------------------------------------
extern const GUID BornoTSFGuidDisplayAttributeInput = {
    0xd5204781,
    0x9e12,
    0x4b78,
    {0xb1, 0x38, 0x7a, 0x69, 0x2c, 0x48, 0x91, 0x10}};

extern const GUID BornoTSFGuidDisplayAttributeConverted = {
    0xd5204782,
    0x9e12,
    0x4b78,
    {0xb1, 0x38, 0x7a, 0x69, 0x2c, 0x48, 0x91, 0x10}};

//---------------------------------------------------------------------
// Unicode byte order mark
//---------------------------------------------------------------------
extern const WCHAR UnicodeByteOrderMark = 0xFEFF;

//---------------------------------------------------------------------
// defined item in setting file table [PreservedKey] section
//---------------------------------------------------------------------
extern const WCHAR ImeModeDescription[] = L"Click to Open Menu";
extern const int ImeModeOnIcoIndex = IME_MODE_ON_ICON_INDEX;
extern const int ImeModeOffIcoIndex = IME_MODE_OFF_ICON_INDEX;
extern const WCHAR LangbarImeModeDescription[] = L"Conversion mode";

//---------------------------------------------------------------------
// defined full width characters for Double/Single byte conversion
//---------------------------------------------------------------------
extern const WCHAR FullWidthCharTable[] = {
    0x3000, 0xFF01, 0xFF02, 0xFF03, 0xFF04, 0xFF05, 0xFF06, 0xFF07, 0xFF08, 0xFF09, 0xFF0A, 0xFF0B, 0xFF0C, 0xFF0D, 0xFF0E, 0xFF0F,
    0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1B, 0xFF1C, 0xFF1D, 0xFF1E, 0xFF1F,
    0xFF20, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F,
    0xFF30, 0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 0xFF3B, 0xFF3C, 0xFF3D, 0xFF3E, 0xFF3F,
    0xFF40, 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F,
    0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 0xFF5B, 0xFF5C, 0xFF5D, 0xFF5E
};

//---------------------------------------------------------------------
// defined full width characters for Double/Single byte conversion
//----------------------------------------------------------------------------

#define TF_MOD_ALLALT     (TF_MOD_RALT | TF_MOD_LALT | TF_MOD_ALT)
#define TF_MOD_ALLCONTROL (TF_MOD_RCONTROL | TF_MOD_LCONTROL | TF_MOD_CONTROL)
#define TF_MOD_ALLSHIFT   (TF_MOD_RSHIFT | TF_MOD_LSHIFT | TF_MOD_SHIFT)
#define TF_MOD_RLALT      (TF_MOD_RALT | TF_MOD_LALT)
#define TF_MOD_RLCONTROL  (TF_MOD_RCONTROL | TF_MOD_LCONTROL)
#define TF_MOD_RLSHIFT    (TF_MOD_RSHIFT | TF_MOD_LSHIFT)

#define CheckMod(m0, m1, mod)        \
    if (m1 & TF_MOD_ ## mod ##)      \
{ \
    if (!(m0 & TF_MOD_ ## mod ##)) \
{      \
    return FALSE;   \
}      \
} \
    else       \
{ \
    if ((m1 ^ m0) & TF_MOD_RL ## mod ##)    \
{      \
    return FALSE;   \
}      \
} \

BOOL CheckModifiers(UINT modCurrent, UINT mod)
{
    mod &= ~TF_MOD_ON_KEYUP;

    if (mod & TF_MOD_IGNORE_ALL_MODIFIER)
    {
        return TRUE;
    }

    if (modCurrent == mod)
    {
        return TRUE;
    }

    if (modCurrent && !mod)
    {
        return FALSE;
    }

    CheckMod(modCurrent, mod, ALT);
    CheckMod(modCurrent, mod, SHIFT);
    CheckMod(modCurrent, mod, CONTROL);

    return TRUE;
}

USHORT ModifiersValue = 0;
BOOL   IsShiftKeyDownOnly = FALSE;
BOOL   IsControlKeyDownOnly = FALSE;
BOOL   IsAltKeyDownOnly = FALSE;

BOOL UpdateModifiers(WPARAM wParam, LPARAM lParam)
{
    SHORT sksMenu = GetKeyState(VK_MENU);
    SHORT sksCtrl = GetKeyState(VK_CONTROL);
    SHORT sksShft = GetKeyState(VK_SHIFT);

    switch (wParam & 0xff)
    {
    case VK_MENU:
        if (sksMenu & 0x8000)
        {
            if (lParam & 0x01000000)
            {
                ModifiersValue |= (TF_MOD_RALT | TF_MOD_ALT);
            }
            else
            {
                ModifiersValue |= (TF_MOD_LALT | TF_MOD_ALT);
            }

            if (!(lParam & 0x40000000))
            {
                if (!(sksCtrl & 0x8000) && !(sksShft & 0x8000))
                {
                    IsAltKeyDownOnly = TRUE;
                }
                else
                {
                    IsShiftKeyDownOnly = FALSE;
                    IsControlKeyDownOnly = FALSE;
                    IsAltKeyDownOnly = FALSE;
                }
            }
        }
        break;

    case VK_CONTROL:
        if (sksCtrl & 0x8000)    
        {
            if (lParam & 0x01000000)
            {
                ModifiersValue |= (TF_MOD_RCONTROL | TF_MOD_CONTROL);
            }
            else
            {
                ModifiersValue |= (TF_MOD_LCONTROL | TF_MOD_CONTROL);
            }

            if (!(lParam & 0x40000000))
            {
                if (!(sksShft & 0x8000) && !(sksMenu & 0x8000))
                {
                    IsControlKeyDownOnly = TRUE;
                }
                else
                {
                    IsShiftKeyDownOnly = FALSE;
                    IsControlKeyDownOnly = FALSE;
                    IsAltKeyDownOnly = FALSE;
                }
            }
        }
        break;

    case VK_SHIFT:
        if (sksShft & 0x8000)    
        {
            if (((lParam >> 16) & 0x00ff) == 0x36)
            {
                ModifiersValue |= (TF_MOD_RSHIFT | TF_MOD_SHIFT);
            }
            else
            {
                ModifiersValue |= (TF_MOD_LSHIFT | TF_MOD_SHIFT);
            }

            if (!(lParam & 0x40000000))
            {
                if (!(sksMenu & 0x8000) && !(sksCtrl & 0x8000))
                {
                    IsShiftKeyDownOnly = TRUE;
                }
                else
                {
                    IsShiftKeyDownOnly = FALSE;
                    IsControlKeyDownOnly = FALSE;
                    IsAltKeyDownOnly = FALSE;
                }
            }
        }
        break;

    default:
        IsShiftKeyDownOnly = FALSE;
        IsControlKeyDownOnly = FALSE;
        IsAltKeyDownOnly = FALSE;
        break;
    }

    if (!(sksMenu & 0x8000))
    {
        ModifiersValue &= ~TF_MOD_ALLALT;
    }
    if (!(sksCtrl & 0x8000))
    {
        ModifiersValue &= ~TF_MOD_ALLCONTROL;
    }
    if (!(sksShft & 0x8000))
    {
        ModifiersValue &= ~TF_MOD_ALLSHIFT;
    }

    return TRUE;
}
}

