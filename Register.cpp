// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "Define.h"


static const WCHAR RegInfo_Prefix_CLSID[] = L"CLSID\\";
static const WCHAR RegInfo_Key_InProSvr32[] = L"InProcServer32";
static const WCHAR RegInfo_Key_ThreadModel[] = L"ThreadingModel";

static const WCHAR TEXTSERVICE_DESC[] = L"BornoTSF Keyboard";

static const GUID SupportCategories[] = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
    GUID_TFCAT_TIPCAP_SECUREMODE,
    GUID_TFCAT_TIPCAP_COMLESS,
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, 
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};

//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

#include "storage/RegKey.h"

struct PROFILE_ITEM {
    const GUID* pGuid;
    LPCWSTR pwszDesc;
    DWORD dwLayoutId;
};

static const PROFILE_ITEM ProfileItems[] = {
    { &Global::BornoTSFGuidProfile, L"BornoTSF - Borno Phonetic", BORNO_PHONETIC },
    { &Global::GuidProfileAvroPhonetic, L"BornoTSF - Avro Phonetic", BORNO_AVRO },
    { &Global::GuidProfileKhipro, L"BornoTSF - Khipro", BORNO_KHIPRO },
    { &Global::GuidProfileJatiyo, L"BornoTSF - Jatiyo", LAYOUT_JATIYO },
    { &Global::GuidProfileProbhat, L"BornoTSF - Probhat", LAYOUT_PROBHAT },
};

BOOL RegisterProfiles()
{
    HRESULT hr = S_FALSE;
    ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfileMgr, (void**)&pITfInputProcessorProfileMgr);
    if (FAILED(hr))
    {
        return FALSE;
    }

    ITfInputProcessorProfiles *pProfiles = nullptr;
    CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles, (void**)&pProfiles);

    WCHAR achIconFile[MAX_PATH] = {'\0'};
    DWORD cchA = GetModuleFileName(Global::dllInstanceHandle, achIconFile, MAX_PATH);
    cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
    achIconFile[cchA] = '\0';

    HKL hklUS = LoadKeyboardLayout(L"00000409", KLF_NOTELLSHELL);

    CRegKey regKey;
    regKey.Open(HKEY_CURRENT_USER, L"Software\\BornoTSF", KEY_READ);

    WCHAR szLayouts[512] = { 0 };
    ULONG cchLayouts = ARRAYSIZE(szLayouts);
    std::wstring enabled = L"1,2,3,5,6";
    if (regKey.GetHKEY() != NULL && regKey.QueryStringValue(L"EnabledLayouts", szLayouts, &cchLayouts) == ERROR_SUCCESS && cchLayouts > 0)
    {
        enabled = szLayouts;
    }

    auto isEnabled = [&](DWORD layoutId) {
        std::wstring search = L"," + enabled + L",";
        std::wstring target = L"," + std::to_wstring(layoutId) + L",";
        return search.find(target) != std::wstring::npos;
    };

    for (int i = 0; i < ARRAYSIZE(ProfileItems); i++)
    {
        BOOL show = isEnabled(ProfileItems[i].dwLayoutId);

        size_t lenOfDesc = 0;
        StringCchLength(ProfileItems[i].pwszDesc, STRSAFE_MAX_CCH, &lenOfDesc);

        hr = pITfInputProcessorProfileMgr->RegisterProfile(Global::BornoTSFCLSID,
            TEXTSERVICE_LANGID,
            *ProfileItems[i].pGuid,
            ProfileItems[i].pwszDesc,
            static_cast<ULONG>(lenOfDesc),
            L"",
            0,
            (ULONG)-1,
            hklUS, 0, TRUE, 0);

        if (pProfiles)
        {
            pProfiles->EnableLanguageProfile(Global::BornoTSFCLSID, TEXTSERVICE_LANGID, *ProfileItems[i].pGuid, show);
        }

        if (FAILED(hr))
        {
            break;
        }
    }

    if (pProfiles)
    {
        pProfiles->Release();
    }

    if (pITfInputProcessorProfileMgr)
    {
        pITfInputProcessorProfileMgr->Release();
    }

    return SUCCEEDED(hr);
}

void UnregisterProfiles()
{
    HRESULT hr = S_OK;
    ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfileMgr, (void**)&pITfInputProcessorProfileMgr);
    if (FAILED(hr))
    {
        return;
    }

    for (int i = 0; i < ARRAYSIZE(ProfileItems); i++)
    {
        pITfInputProcessorProfileMgr->UnregisterProfile(Global::BornoTSFCLSID, TEXTSERVICE_LANGID, *ProfileItems[i].pGuid, 0);
    }

    if (pITfInputProcessorProfileMgr)
    {
        pITfInputProcessorProfileMgr->Release();
    }
}

BOOL UpdateProfileRegistration(REFGUID guidProfile, BOOL enable)
{
    ITfInputProcessorProfiles *pProfiles = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles, (void**)&pProfiles);
    if (SUCCEEDED(hr) && pProfiles != nullptr)
    {
        pProfiles->EnableLanguageProfile(Global::BornoTSFCLSID, TEXTSERVICE_LANGID, guidProfile, enable);
        pProfiles->Release();
        return TRUE;
    }
    return FALSE;
}



//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL RegisterCategories()
{
    ITfCategoryMgr* pCategoryMgr = nullptr;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
    if (FAILED(hr))
    {
        return FALSE;
    }

    for each(GUID guid in SupportCategories)
    {
        hr = pCategoryMgr->RegisterCategory(Global::BornoTSFCLSID, guid, Global::BornoTSFCLSID);
    }

    pCategoryMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterCategories
//
//----------------------------------------------------------------------------

void UnregisterCategories()
{
    ITfCategoryMgr* pCategoryMgr = nullptr;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
    if (FAILED(hr))
    {
        return;
    }

    for each(GUID guid in SupportCategories)
    {
        pCategoryMgr->UnregisterCategory(Global::BornoTSFCLSID, guid, Global::BornoTSFCLSID);
    }
  
    pCategoryMgr->Release();

    return;
}

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
//----------------------------------------------------------------------------

LONG RecurseDeleteKey(_In_ HKEY hParentKey, _In_ LPCTSTR lpszKey)
{
    HKEY regKeyHandle = nullptr;
    LONG res = 0;
    FILETIME time;
    WCHAR stringBuffer[256] = {'\0'};
    DWORD size = ARRAYSIZE(stringBuffer);

    if (RegOpenKey(hParentKey, lpszKey, &regKeyHandle) != ERROR_SUCCESS)
    {
        return ERROR_SUCCESS;
    }

    res = ERROR_SUCCESS;
    while (RegEnumKeyEx(regKeyHandle, 0, stringBuffer, &size, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
    {
        stringBuffer[ARRAYSIZE(stringBuffer)-1] = '\0';
        res = RecurseDeleteKey(regKeyHandle, stringBuffer);
        if (res != ERROR_SUCCESS)
        {
            break;
        }
        size = ARRAYSIZE(stringBuffer);
    }
    RegCloseKey(regKeyHandle);

    return res == ERROR_SUCCESS ? RegDeleteKey(hParentKey, lpszKey) : res;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL RegisterServer()
{
    DWORD copiedStringLen = 0;
    HKEY regKeyHandle = nullptr;
    HKEY regSubkeyHandle = nullptr;
    BOOL ret = FALSE;
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = {'\0'};
    WCHAR achFileName[MAX_PATH] = {'\0'};

    if (!CLSIDToString(Global::BornoTSFCLSID, achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1))
    {
        return FALSE;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regKeyHandle, &copiedStringLen) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(regKeyHandle, NULL, 0, REG_SZ, (const BYTE *)TEXTSERVICE_DESC, (_countof(TEXTSERVICE_DESC))*sizeof(WCHAR)) != ERROR_SUCCESS)
        {
            goto Exit;
        }

        if (RegCreateKeyEx(regKeyHandle, RegInfo_Key_InProSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regSubkeyHandle, &copiedStringLen) == ERROR_SUCCESS)
        {
            copiedStringLen = GetModuleFileNameW(Global::dllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
            copiedStringLen = (copiedStringLen >= (MAX_PATH - 1)) ? MAX_PATH : (++copiedStringLen);
            if (RegSetValueEx(regSubkeyHandle, NULL, 0, REG_SZ, (const BYTE *)achFileName, (copiedStringLen)*sizeof(WCHAR)) != ERROR_SUCCESS)
            {
                goto Exit;
            }
            if (RegSetValueEx(regSubkeyHandle, RegInfo_Key_ThreadModel, 0, REG_SZ, (const BYTE *)TEXTSERVICE_MODEL, (_countof(TEXTSERVICE_MODEL)) * sizeof(WCHAR)) != ERROR_SUCCESS)
            {
                goto Exit;
            }

            ret = TRUE;
        }
    }

Exit:
    if (regSubkeyHandle)
    {
        RegCloseKey(regSubkeyHandle);
        regSubkeyHandle = nullptr;
    }
    if (regKeyHandle)
    {
        RegCloseKey(regKeyHandle);
        regKeyHandle = nullptr;
    }

    return ret;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void UnregisterServer()
{
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = {'\0'};

    if (!CLSIDToString(Global::BornoTSFCLSID, achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1))
    {
        return;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}
