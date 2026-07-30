// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "BornoTSF.h"
#include <ShlObj.h>

#include <cwctype>
#include <locale>
#include <regex>
#include <string>

#include "Compartment.h"
#include "Globals.h"
#include "Settings.h"
#include "LanguageBar.h"
#include "RegKey.h"
#include "TfInputProcessorProfile.h"
#include "CompositionProcessorEngine.h"

//////////////////////////////////////////////////////////////////////
//
// BornoTSF implementation.
//
//////////////////////////////////////////////////////////////////////

// Called from OnActivated — guidProfile and langid come directly from TSF.
BOOL BornoTSF::_AddTextProcessorEngine(LANGID langid, REFGUID guidProfile) {
  if (_pCompositionProcessorEngine == nullptr) {
    _pCompositionProcessorEngine =
        new (std::nothrow) CCompositionProcessorEngine();
  }
  if (!_pCompositionProcessorEngine) {
    return FALSE;
  }

  // Check if already set up with the same profile (no-op on duplicate activate).
  if (_pCompositionProcessorEngine != nullptr) {
    LANGID langidProfile = 0;
    GUID guidLanguageProfile =
        _pCompositionProcessorEngine->GetLanguageProfile(&langidProfile);
    if ((langid == langidProfile) &&
        IsEqualGUID(guidProfile, guidLanguageProfile)) {
      return TRUE;
    }
  }

  if (FALSE == _pCompositionProcessorEngine->SetupLanguageProfile(
                   langid, guidProfile, _GetThreadMgr(), _GetClientId(),
                   _IsSecureMode(), _IsComLess())) {
    return FALSE;
  }

  return TRUE;
}

// Called from ActivateEx at startup — we don't know which profile is active
// yet, so we query. This may fail (return TRUE early) before OnActivated fires.
BOOL BornoTSF::_AddTextProcessorEngine() {
  LANGID langid = 0;
  GUID guidProfile = GUID_NULL;

  CTfInputProcessorProfile profile;
  if (FAILED(profile.CreateInstance())) {
    return FALSE;
  }

  // GetActiveLanguageProfile may fail here if Windows hasn't activated our
  // profile yet (normal on first launch). OnActivated will call the two-arg
  // overload later with the correct GUID.
  if (FAILED(profile.GetActiveLanguageProfile(Global::BornoTSFCLSID, &langid, &guidProfile))) {
    return TRUE;  // Not an error — OnActivated will set things up.
  }

  return _AddTextProcessorEngine(langid, guidProfile);
}



//////////////////////////////////////////////////////////////////////
//
// CompositionProcessorEngine implementation.
//
//////////////////////////////////////////////////////////////////////

CCompositionProcessorEngine::CCompositionProcessorEngine() {
  _langid = 0xffff;
  _guidProfile = GUID_NULL;
  _tfClientId = TF_CLIENTID_NULL;

  _pLanguageBar_IMEMode = nullptr;

  _pCompartmentConversion = nullptr;
  _pCompartmentKeyboardOpenEventSink = nullptr;
  _pCompartmentConversionEventSink = nullptr;
  _mappedBuffer = nullptr;

  InitKeyStrokeTable();
}

CCompositionProcessorEngine::~CCompositionProcessorEngine() {
  if (_pLanguageBar_IMEMode) {
    _pLanguageBar_IMEMode->CleanUp();
    _pLanguageBar_IMEMode->Release();
    _pLanguageBar_IMEMode = nullptr;
  }

  if (_mappedBuffer) {
    delete[] _mappedBuffer;
    _mappedBuffer = nullptr;
  }

  if (_pCompartmentConversion) {
    delete _pCompartmentConversion;
    _pCompartmentConversion = nullptr;
  }
  if (_pCompartmentKeyboardOpenEventSink) {
    _pCompartmentKeyboardOpenEventSink->_Unadvise();
    delete _pCompartmentKeyboardOpenEventSink;
    _pCompartmentKeyboardOpenEventSink = nullptr;
  }
  if (_pCompartmentConversionEventSink) {
    _pCompartmentConversionEventSink->_Unadvise();
    delete _pCompartmentConversionEventSink;
    _pCompartmentConversionEventSink = nullptr;
  }
}

BOOL CCompositionProcessorEngine::SetupLanguageProfile(
    LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr,
    TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) {
  BOOL ret = TRUE;
  if ((tfClientId == 0) && (pThreadMgr == nullptr)) {
    ret = FALSE;
    goto Exit;
  }

  _isComLessMode = isComLessMode;
  _langid = langid;
  _guidProfile = guidLanguageProfile;
  _tfClientId = tfClientId;

  if (IsEqualGUID(_guidProfile, Global::BornoTSFGuidProfile))
  {
      Settings::SetCurrentLayout(BORNO_PHONETIC);
  }
  else if (IsEqualGUID(_guidProfile, Global::GuidProfileAvroPhonetic))
  {
      Settings::SetCurrentLayout(BORNO_AVRO);
  }
  else if (IsEqualGUID(_guidProfile, Global::GuidProfileKhipro))
  {
      Settings::SetCurrentLayout(BORNO_KHIPRO);
  }
  else if (IsEqualGUID(_guidProfile, Global::GuidProfileJatiyo))
  {
      Settings::SetCurrentLayout(LAYOUT_JATIYO);
  }
  else if (IsEqualGUID(_guidProfile, Global::GuidProfileProbhat))
  {
      Settings::SetCurrentLayout(LAYOUT_PROBHAT);
  }

  SetupPreserved(pThreadMgr, tfClientId);
  InitializeBornoTSFCompartment(pThreadMgr, tfClientId);
  SetupPunctuationPair();
  SetupLanguageBar(pThreadMgr, tfClientId, isSecureMode);

  SetupKeystroke();
  SetupConfiguration();

Exit:
  return ret;
}

BOOL CCompositionProcessorEngine::AddVirtualKey(WCHAR wch) {
  if (!wch) {
    return FALSE;
  }

  DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();
  PWCHAR pwch = new (std::nothrow) WCHAR[srgKeystrokeBufLen + 1];
  if (!pwch) {
    return FALSE;
  }

  memcpy(pwch, _keystrokeBuffer.Get(), srgKeystrokeBufLen * sizeof(WCHAR));
  pwch[srgKeystrokeBufLen] = wch;

  if (_keystrokeBuffer.Get()) {
    delete[] _keystrokeBuffer.Get();
  }

  _keystrokeBuffer.Set(pwch, srgKeystrokeBufLen + 1);

  return TRUE;
}

void CCompositionProcessorEngine::RemoveVirtualKey(DWORD_PTR dwIndex) {
  DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();

  if (dwIndex + 1 < srgKeystrokeBufLen) {
    memmove((::BYTE *)_keystrokeBuffer.Get() + (dwIndex * sizeof(WCHAR)),
            (::BYTE *)_keystrokeBuffer.Get() + ((dwIndex + 1) * sizeof(WCHAR)),
            (srgKeystrokeBufLen - dwIndex - 1) * sizeof(WCHAR));
  }

  _keystrokeBuffer.Set(_keystrokeBuffer.Get(), srgKeystrokeBufLen - 1);
}

void CCompositionProcessorEngine::PurgeVirtualKey() {
  if (_keystrokeBuffer.Get()) {
    delete[] _keystrokeBuffer.Get();
    _keystrokeBuffer.Set(NULL, 0);
  }
  if (_mappedBuffer) {
    delete[] _mappedBuffer;
    _mappedBuffer = nullptr;
  }
}

WCHAR CCompositionProcessorEngine::GetVirtualKey(DWORD_PTR dwIndex) {
  if (dwIndex < _keystrokeBuffer.GetLength()) {
    return *(_keystrokeBuffer.Get() + dwIndex);
  }
  return 0;
}

std::wstring CCompositionProcessorEngine::ApplyAllRules(const std::wstring &input) {
  std::wstring output = input;
  std::string utf8Input = WstringToString(input, CP_UTF8);

  switch (Global::currentLayout) {
    case BORNO_PHONETIC:
      output = StringToWstring(bornoPhonetic.parse(utf8Input), CP_UTF8);
      break;
    case BORNO_AVRO:
      output = StringToWstring(avroPhonetic.Convert(utf8Input), CP_UTF8);
      break;
    case BORNO_KHIPRO:
      output = StringToWstring(khipro.transliterate(utf8Input), CP_UTF8);
      break;
    case LAYOUT_JATIYO:
      for (const auto &rule : jatiyoLayout.GetRules()) {
        output = boost::regex_replace(output, rule.first, rule.second);
      }
      break;
    case LAYOUT_PROBHAT:
      for (const auto &rule : probhatLayout.GetRules()) {
        output = boost::regex_replace(output, rule.first, rule.second);
      }
      break;
    default:
      output = StringToWstring(bornoPhonetic.parse(utf8Input), CP_UTF8);
      break;
  }

  return output;
}

std::wstring CCompositionProcessorEngine::StringToWstring(const std::string &str, UINT codePage) {
  if (str.empty()) return std::wstring();
  int size_needed = MultiByteToWideChar(codePage, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(codePage, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}


std::wstring CCompositionProcessorEngine::WstringToString(const WCHAR *ptr, size_t len) {
  if (!ptr || len == 0) return L"";
  return std::wstring(ptr, len);
}

std::string CCompositionProcessorEngine::WstringToString(const std::wstring &wstr, UINT codePage) {
  if (wstr.empty()) return std::string();
  int size_needed = WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
  return strTo;
}

WCHAR *CCompositionProcessorEngine::getMappedBuffer(CStringRange keystrokeBuffer) {
  if (keystrokeBuffer.GetLength() == 0) {
    return nullptr;
  }

  if (_mappedBuffer) {
    delete[] _mappedBuffer;
    _mappedBuffer = nullptr;
  }

  std::wstring inputStr(keystrokeBuffer.Get(), keystrokeBuffer.GetLength());
  std::wstring mappedStr = ApplyAllRules(inputStr);

  size_t len = mappedStr.length();
  _mappedBuffer = new (std::nothrow) WCHAR[len + 1];
  if (!_mappedBuffer) {
    return nullptr;
  }
  wcscpy_s(_mappedBuffer, len + 1, mappedStr.c_str());

  return _mappedBuffer;
}

void CCompositionProcessorEngine::GetReadingStrings(
    _Inout_ BornoTSFArray<CStringRange> *pReadingStrings) {
  if (!pReadingStrings) return;
  pReadingStrings->Clear();

  if (_keystrokeBuffer.GetLength() == 0) {
    return;
  }

  WCHAR *pMapped = getMappedBuffer(_keystrokeBuffer);
  if (pMapped) {
    CStringRange *pRange = pReadingStrings->Append();
    if (pRange) {
      pRange->Set(pMapped, (DWORD)wcslen(pMapped));
    }
  }
}

HRESULT BornoTSF::CreateInstance(REFCLSID rclsid, REFIID riid,
                                 _Outptr_result_maybenull_ LPVOID *ppv,
                                 _Out_opt_ HINSTANCE *phInst,
                                 BOOL isComLessMode) {
  HRESULT hr = S_OK;
  if (phInst == nullptr) {
    return E_INVALIDARG;
  }

  *phInst = nullptr;

  if (!isComLessMode) {
    hr = ::CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, riid, ppv);
  } else {
    hr = BornoTSF::ComLessCreateInstance(rclsid, riid, ppv, phInst);
  }

  return hr;
}

HRESULT BornoTSF::ComLessCreateInstance(REFGUID rclsid, REFIID riid,
                                        _Outptr_result_maybenull_ void **ppv,
                                        _Out_opt_ HINSTANCE *phInst) {
  HRESULT hr = S_OK;
  HINSTANCE BornoTSFDllHandle = nullptr;
  WCHAR wchPath[MAX_PATH] = {'\0'};
  WCHAR szExpandedPath[MAX_PATH] = {'\0'};
  DWORD dwCnt = 0;
  *ppv = nullptr;

  hr = phInst ? S_OK : E_FAIL;
  if (SUCCEEDED(hr)) {
    *phInst = nullptr;
    hr = BornoTSF::GetComModuleName(rclsid, wchPath, ARRAYSIZE(wchPath));
    if (SUCCEEDED(hr)) {
      dwCnt = ExpandEnvironmentStringsW(wchPath, szExpandedPath,
                                        ARRAYSIZE(szExpandedPath));
      hr = (0 < dwCnt && dwCnt <= ARRAYSIZE(szExpandedPath)) ? S_OK : E_FAIL;
      if (SUCCEEDED(hr)) {
        BornoTSFDllHandle = LoadLibraryEx(szExpandedPath, NULL, 0);
        hr = BornoTSFDllHandle ? S_OK : E_FAIL;
        if (SUCCEEDED(hr)) {
          *phInst = BornoTSFDllHandle;
          FARPROC pfn = GetProcAddress(BornoTSFDllHandle, "DllGetClassObject");
          hr = pfn ? S_OK : E_FAIL;
          if (SUCCEEDED(hr)) {
            IClassFactory *pClassFactory = nullptr;
            hr = ((HRESULT(STDAPICALLTYPE *)(REFCLSID rclsid, REFIID riid,
                                             LPVOID * ppv))(pfn))(
                rclsid, IID_IClassFactory, (void **)&pClassFactory);
            if (SUCCEEDED(hr) && pClassFactory) {
              hr = pClassFactory->CreateInstance(NULL, riid, ppv);
              pClassFactory->Release();
            }
          }
        }
      }
    }
  }

  if (!SUCCEEDED(hr) && phInst && *phInst) {
    FreeLibrary(*phInst);
    *phInst = 0;
  }
  return hr;
}

HRESULT BornoTSF::GetComModuleName(REFGUID rclsid,
                                   _Out_writes_(cchPath) WCHAR *wchPath,
                                   DWORD cchPath) {
  HRESULT hr = S_OK;

  CRegKey key;
  WCHAR wchClsid[CLSID_STRLEN + 1];
  hr = CLSIDToString(rclsid, wchClsid) ? S_OK : E_FAIL;
  if (SUCCEEDED(hr)) {
    WCHAR wchKey[MAX_PATH];
    hr = StringCchPrintfW(wchKey, ARRAYSIZE(wchKey),
                          L"CLSID\\%s\\InProcServer32", wchClsid);
    if (SUCCEEDED(hr)) {
      hr = (key.Open(HKEY_CLASSES_ROOT, wchKey, KEY_READ) == ERROR_SUCCESS)
               ? S_OK
               : E_FAIL;
      if (SUCCEEDED(hr)) {
        WCHAR wszModel[MAX_PATH];
        ULONG cch = ARRAYSIZE(wszModel);
        hr = (key.QueryStringValue(L"ThreadingModel", wszModel, &cch) ==
              ERROR_SUCCESS)
                 ? S_OK
                 : E_FAIL;
        if (SUCCEEDED(hr)) {
          if (CompareStringOrdinal(wszModel, -1, L"Apartment", -1, TRUE) ==
              CSTR_EQUAL) {
            hr =
                (key.QueryStringValue(NULL, wchPath, &cchPath) == ERROR_SUCCESS)
                    ? S_OK
                    : E_FAIL;
          } else {
            hr = E_FAIL;
          }
        }
      }
    }
  }

  return hr;
}

void CCompositionProcessorEngine::InitKeyStrokeTable() {
  memset(_keystrokeTable, 0, sizeof(_keystrokeTable));

  struct KeyMap {
    UINT vkey;
    UINT modifier;
  };

  const KeyMap keys[] = {
      {VK_OEM_3, 0},      {VK_OEM_3, TF_MOD_SHIFT},
      {'1', 0},           {'1', TF_MOD_SHIFT},
      {'2', 0},
      {'3', 0},
      {'4', 0},           {'4', TF_MOD_SHIFT},
      {'5', 0},           {'5', TF_MOD_SHIFT},
      {'6', 0},           {'6', TF_MOD_SHIFT},
      {'7', 0},           {'7', TF_MOD_SHIFT},
      {'8', 0},           {'8', TF_MOD_SHIFT},
      {'9', 0},           {'9', TF_MOD_SHIFT},
      {'0', 0},           {'0', TF_MOD_SHIFT},
      {VK_OEM_MINUS, 0},  {VK_OEM_MINUS, TF_MOD_SHIFT},
      {VK_OEM_PLUS, 0},   {VK_OEM_PLUS, TF_MOD_SHIFT},
      {'Q', 0},           {'Q', TF_MOD_SHIFT},
      {'W', 0},           {'W', TF_MOD_SHIFT},
      {'E', 0},           {'E', TF_MOD_SHIFT},
      {'R', 0},           {'R', TF_MOD_SHIFT},
      {'T', 0},           {'T', TF_MOD_SHIFT},
      {'Y', 0},           {'Y', TF_MOD_SHIFT},
      {'U', 0},           {'U', TF_MOD_SHIFT},
      {'I', 0},           {'I', TF_MOD_SHIFT},
      {'O', 0},           {'O', TF_MOD_SHIFT},
      {'P', 0},           {'P', TF_MOD_SHIFT},
      {VK_OEM_4, 0},      {VK_OEM_4, TF_MOD_SHIFT},
      {VK_OEM_6, 0},      {VK_OEM_6, TF_MOD_SHIFT},
      {VK_OEM_5, 0},      {VK_OEM_5, TF_MOD_SHIFT},
      {'A', 0},           {'A', TF_MOD_SHIFT},
      {'S', 0},           {'S', TF_MOD_SHIFT},
      {'D', 0},           {'D', TF_MOD_SHIFT},
      {'F', 0},           {'F', TF_MOD_SHIFT},
      {'G', 0},           {'G', TF_MOD_SHIFT},
      {'H', 0},           {'H', TF_MOD_SHIFT},
      {'J', 0},           {'J', TF_MOD_SHIFT},
      {'K', 0},           {'K', TF_MOD_SHIFT},
      {'L', 0},           {'L', TF_MOD_SHIFT},
      {VK_OEM_1, 0},      {VK_OEM_1, TF_MOD_SHIFT},
      {VK_OEM_7, 0},      {VK_OEM_7, TF_MOD_SHIFT},
      {'Z', 0},           {'Z', TF_MOD_SHIFT},
      {'X', 0},           {'X', TF_MOD_SHIFT},
      {'C', 0},           {'C', TF_MOD_SHIFT},
      {'V', 0},           {'V', TF_MOD_SHIFT},
      {'B', 0},           {'B', TF_MOD_SHIFT},
      {'N', 0},           {'N', TF_MOD_SHIFT},
      {'M', 0},           {'M', TF_MOD_SHIFT},
      {VK_OEM_COMMA, 0},  {VK_OEM_COMMA, TF_MOD_SHIFT},
      {VK_OEM_PERIOD, 0}, {VK_OEM_PERIOD, TF_MOD_SHIFT},
      {VK_OEM_2, 0},      {VK_OEM_2, TF_MOD_SHIFT}
  };

  const int keyCount = sizeof(keys) / sizeof(keys[0]);

  for (int i = 0; i < keyCount; i++) {
    _keystrokeTable[i].VirtualKey = keys[i].vkey;
    _keystrokeTable[i].Modifiers = keys[i].modifier;
    _keystrokeTable[i].Function = FUNCTION_INPUT;
  }
}

void CCompositionProcessorEngine::ShowAllLanguageBarIcons() {
  SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, FALSE);
  if (_pLanguageBar_IMEMode) {
    _pLanguageBar_IMEMode->UpdateIcon();
  }
}

void CCompositionProcessorEngine::HideAllLanguageBarIcons() {
  SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, TRUE);
}

BOOL CCompositionProcessorEngine::IsVirtualKeyNeed(
    UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing,
    _Out_opt_ _KEYSTROKE_STATE *pKeyState) {
  pwch;
  if (pKeyState) {
    pKeyState->Category = CATEGORY_NONE;
    pKeyState->Function = FUNCTION_NONE;
  }

  if (fComposing) {
    if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE)) {
      return TRUE;
    }
  }

  if (!fComposing) {
    if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT)) {
      return TRUE;
    }
  }

  if (fComposing) {
    switch (uCode) {
      case VK_SPACE:
        if (pKeyState) {
          pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
          pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
        }
        return FALSE;  
      case VK_RETURN:
      case VK_CONTROL:
        if (pKeyState) {
          pKeyState->Category = CATEGORY_COMPOSING;
          pKeyState->Function = FUNCTION_CONVERT;
        }
        return TRUE;
      case VK_ESCAPE:
        if (pKeyState) {
          pKeyState->Category = CATEGORY_COMPOSING;
          pKeyState->Function = FUNCTION_CANCEL;
        }
        return TRUE;
      case VK_BACK:
        if (pKeyState) {
          pKeyState->Category = CATEGORY_COMPOSING;
          pKeyState->Function = FUNCTION_BACKSPACE;
        }
        return TRUE;
      case VK_LEFT:
      case VK_RIGHT:
      case VK_UP:
      case VK_DOWN:
      case VK_PRIOR:
      case VK_NEXT:
      case VK_HOME:
      case VK_END:
        if (pKeyState) {
          pKeyState->Category = CATEGORY_COMPOSING;
          pKeyState->Function = FUNCTION_CONVERT;
        }
        return TRUE;
    }
  }

  return FALSE;
}

BOOL CCompositionProcessorEngine::IsVirtualKeyKeystrokeComposition(
    UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState,
    KEYSTROKE_FUNCTION function) {
  if (pKeyState == nullptr) {
    return FALSE;
  }

  pKeyState->Category = CATEGORY_NONE;
  pKeyState->Function = FUNCTION_NONE;

  for (UINT i = 0; i < _KeystrokeComposition.Count(); i++) {
    _KEYSTROKE *pKeystroke = nullptr;

    pKeystroke = _KeystrokeComposition.GetAt(i);

    if ((pKeystroke->VirtualKey == uCode) &&
        Global::CheckModifiers(Global::ModifiersValue, pKeystroke->Modifiers)) {
      if (function == FUNCTION_NONE) {
        pKeyState->Category = CATEGORY_COMPOSING;
        pKeyState->Function = pKeystroke->Function;
        return TRUE;
      } else if (function == pKeystroke->Function) {
        pKeyState->Category = CATEGORY_COMPOSING;
        pKeyState->Function = pKeystroke->Function;
        return TRUE;
      }
    }
  }

  return FALSE;
}


void CCompositionProcessorEngine::SetupKeystroke() {
  _KeystrokeComposition.Clear();

  for (int i = 0; i < 100; i++) {
    if (_keystrokeTable[i].VirtualKey != 0) {
      _KEYSTROKE *pKeystroke = _KeystrokeComposition.Append();
      if (pKeystroke) {
        *pKeystroke = _keystrokeTable[i];
      }
    }
  }
}

void CCompositionProcessorEngine::SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) {
  TF_PRESERVEDKEY tfPreservedKey;
  tfPreservedKey.uVKey = VK_SPACE;
  tfPreservedKey.uModifiers = TF_MOD_CONTROL;

  SetPreservedKey(Global::BornoTSFGuidImeModePreserveKey, tfPreservedKey, Global::ImeModeDescription, &_PreservedKey_IMEMode);
  InitPreservedKey(&_PreservedKey_IMEMode, pThreadMgr, tfClientId);
}

void CCompositionProcessorEngine::SetupConfiguration() {
}

void CCompositionProcessorEngine::SetupLanguageBar(
    _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode) {
  DWORD dwEnable = 1;
  CreateLanguageBarButton(
      dwEnable, GUID_LBI_INPUTMODE, Global::LangbarImeModeDescription,
      Global::ImeModeDescription, Global::ImeModeOnIcoIndex,
      Global::ImeModeOffIcoIndex, &_pLanguageBar_IMEMode, isSecureMode);

  InitLanguageBar(_pLanguageBar_IMEMode, pThreadMgr, tfClientId,
                  GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);

  _pCompartmentConversion = new (std::nothrow) CCompartment(
      pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
  _pCompartmentKeyboardOpenEventSink =
      new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
  _pCompartmentConversionEventSink =
      new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);

  if (_pCompartmentKeyboardOpenEventSink) {
    _pCompartmentKeyboardOpenEventSink->_Advise(
        pThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);

    CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId,
                                         GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);
  }
  if (_pCompartmentConversionEventSink) {
    _pCompartmentConversionEventSink->_Advise(
        pThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
  }
}

void CCompositionProcessorEngine::CreateLanguageBarButton(
    DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue,
    _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex,
    _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton,
    BOOL isSecureMode) {
  dwEnable;

  if (ppLangBarItemButton) {
    *ppLangBarItemButton = new (std::nothrow)
        CLangBarItemButton(guidLangBar, pwszDescriptionValue, pwszTooltipValue,
                           dwOnIconIndex, dwOffIconIndex, isSecureMode);
  }
}

BOOL CCompositionProcessorEngine::InitLanguageBar(
    _In_ CLangBarItemButton *pLangBarItemButton, _In_ ITfThreadMgr *pThreadMgr,
    TfClientId tfClientId, REFGUID guidCompartment) {
  if (pLangBarItemButton) {
    if (pLangBarItemButton->_AddItem(pThreadMgr) == S_OK) {
      if (pLangBarItemButton->_RegisterCompartment(pThreadMgr, tfClientId,
                                                   guidCompartment)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

void CCompositionProcessorEngine::SetupPunctuationPair() {
  _PunctuationPair.Clear();
}

void CCompositionProcessorEngine::InitializeBornoTSFCompartment(
    _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) {
  CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId,
                                       GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
  CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);

  CCompartment CompartmentConversion(pThreadMgr, tfClientId,
                                     GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
  CompartmentConversion._SetCompartmentDWORD(TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_ALPHANUMERIC);
}

void CCompositionProcessorEngine::SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey) {
  if (!pXPreservedKey) return;
  pXPreservedKey->Guid = clsid;

  TF_PRESERVEDKEY *pPsvKey = pXPreservedKey->TSFPreservedKeyTable.Append();
  if (pPsvKey) {
    *pPsvKey = tfPreservedKey;
  }

  size_t srgKeystrokeBufLen = 0;
  if (StringCchLength(pwszDescription, STRSAFE_MAX_CCH, &srgKeystrokeBufLen) != S_OK) return;
  srgKeystrokeBufLen++;

  pXPreservedKey->Description = new (std::nothrow) WCHAR[srgKeystrokeBufLen];
  if (!pXPreservedKey->Description) return;

  StringCchCopy((LPWSTR)pXPreservedKey->Description, srgKeystrokeBufLen, pwszDescription);
}

BOOL CCompositionProcessorEngine::InitPreservedKey(
    _In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr,
    TfClientId tfClientId) {
  ITfKeystrokeMgr *pKeystrokeMgr = nullptr;

  if (IsEqualGUID(pXPreservedKey->Guid, GUID_NULL)) {
    return FALSE;
  }

  if (pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr,
                                 (void **)&pKeystrokeMgr) != S_OK) {
    return FALSE;
  }

  for (UINT i = 0; i < pXPreservedKey->TSFPreservedKeyTable.Count(); i++) {
    TF_PRESERVEDKEY preservedKey =
        *pXPreservedKey->TSFPreservedKeyTable.GetAt(i);
    preservedKey.uModifiers &= 0xffff;

    size_t lenOfDesc = 0;
    if (StringCchLength(pXPreservedKey->Description, STRSAFE_MAX_CCH,
                        &lenOfDesc) != S_OK) {
      return FALSE;
    }
    pKeystrokeMgr->PreserveKey(tfClientId, pXPreservedKey->Guid, &preservedKey,
                               pXPreservedKey->Description,
                               static_cast<ULONG>(lenOfDesc));
  }

  pKeystrokeMgr->Release();

  return TRUE;
}

BOOL CCompositionProcessorEngine::CheckShiftKeyOnly(
    _In_ BornoTSFArray<TF_PRESERVEDKEY> *pTSFPreservedKeyTable) {
  for (UINT i = 0; i < pTSFPreservedKeyTable->Count(); i++) {
    TF_PRESERVEDKEY *ptfPsvKey = pTSFPreservedKeyTable->GetAt(i);

    if (((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_SHIFT_ONLY & 0xffff0000)) &&
         !Global::IsShiftKeyDownOnly) ||
        ((ptfPsvKey->uModifiers &
          (_TF_MOD_ON_KEYUP_CONTROL_ONLY & 0xffff0000)) &&
         !Global::IsControlKeyDownOnly) ||
        ((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_ALT_ONLY & 0xffff0000)) &&
         !Global::IsAltKeyDownOnly)) {
      return FALSE;
    }
  }

  return TRUE;
}

void CCompositionProcessorEngine::OnPreservedKey(REFGUID rguid,
                                                 _Out_ BOOL *pIsEaten,
                                                 _In_ ITfThreadMgr *pThreadMgr,
                                                 TfClientId tfClientId) {
  if (IsEqualGUID(rguid, _PreservedKey_IMEMode.Guid)) {
    if (!CheckShiftKeyOnly(&_PreservedKey_IMEMode.TSFPreservedKeyTable)) {
      *pIsEaten = FALSE;
      return;
    }
    BOOL isOpen = FALSE;
    CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId,
                                         GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CompartmentKeyboardOpen._GetCompartmentBOOL(isOpen);
    CompartmentKeyboardOpen._SetCompartmentBOOL(isOpen ? FALSE : TRUE);

    *pIsEaten = TRUE;
  } else {
    *pIsEaten = FALSE;
  }
}

void CCompositionProcessorEngine::SetLanguageBarStatus(DWORD status, BOOL isSet) {
  if (_pLanguageBar_IMEMode) {
    _pLanguageBar_IMEMode->SetStatus(status, isSet);
  }
}

void CCompositionProcessorEngine::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) {
  pThreadMgr;
}

void CCompositionProcessorEngine::PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr) {
  pThreadMgr;
}

void CCompositionProcessorEngine::KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) {
  pThreadMgr;
}

HRESULT CCompositionProcessorEngine::CompartmentCallback(_In_ void *pv, REFGUID guidCompartment) {
  guidCompartment;
  CCompositionProcessorEngine *pEngine = (CCompositionProcessorEngine *)pv;
  if (pEngine) {
  }
  return S_OK;
}

CCompositionProcessorEngine::XPreservedKey::XPreservedKey() {
  Guid = GUID_NULL;
  Description = nullptr;
}

CCompositionProcessorEngine::XPreservedKey::~XPreservedKey() {
  if (Description) {
    delete[] Description;
    Description = nullptr;
  }
}

BOOL CCompositionProcessorEngine::XPreservedKey::UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr) {
  ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
  if (pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK) {
    return FALSE;
  }
  for (UINT i = 0; i < TSFPreservedKeyTable.Count(); i++) {
    pKeystrokeMgr->UnpreserveKey(Guid, TSFPreservedKeyTable.GetAt(i));
  }
  pKeystrokeMgr->Release();
  return TRUE;
}

BOOL CCompositionProcessorEngine::IsPunctuation(WCHAR wch) {
  for (UINT i = 0; i < _PunctuationPair.Count(); i++) {
    if (_PunctuationPair.GetAt(i)->_punctuation._Code == wch) {
      return TRUE;
    }
  }
  if (wch == L'.' || wch == L',' || wch == L'!' || wch == L'?' ||
      wch == L';' || wch == L':' || wch == L'|' || wch == L'\\') {
    return TRUE;
  }
  return FALSE;
}

WCHAR CCompositionProcessorEngine::GetPunctuation(WCHAR wch) {
  for (UINT i = 0; i < _PunctuationPair.Count(); i++) {
    CPunctuationPair* pPair = _PunctuationPair.GetAt(i);
    if (pPair->_punctuation._Code == wch) {
      if (pPair->_isPairToggle) {
        pPair->_isPairToggle = FALSE;
        return pPair->_pairPunctuation;
      } else {
        pPair->_isPairToggle = TRUE;
        return pPair->_punctuation._Punctuation;
      }
    }
  }
  return wch;
}

BOOL CCompositionProcessorEngine::IsDoubleSingleByte(WCHAR wch) {
  return (wch >= 0x0021 && wch <= 0x007E) || (wch >= 0xFF01 && wch <= 0xFF5E);
}