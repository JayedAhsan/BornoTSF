// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "BornoTSF.h"

#include "LanguageBar.h"
#include "Globals.h"
#include "Compartment.h"
#include "Settings.h"
#include "resource.h"
#include "storage/RegKey.h"
#include <sstream>
#include <uxtheme.h>
#include <shellapi.h>
#include "CompositionProcessorEngine.h"

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "shell32.lib")

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        EnableThemeDialogTexture(hDlg, ETDT_ENABLETAB);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BTN_GITHUB)
        {
            ShellExecuteW(NULL, L"open", L"https://github.com/codepotro/bornoTSF", NULL, NULL, SW_SHOWNORMAL);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDC_BTN_CHECK_UPDATES)
        {
            ShellExecuteW(NULL, L"open", L"https://codepotro.com/borno/", NULL, NULL, SW_SHOWNORMAL);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        EnableThemeDialogTexture(hDlg, ETDT_ENABLETAB);

        CheckDlgButton(hDlg, IDC_CHK_BORNO_PHONETIC, Settings::IsLayoutEnabled(BORNO_PHONETIC) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_AVRO_PHONETIC, Settings::IsLayoutEnabled(BORNO_AVRO) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_KHIPRO, Settings::IsLayoutEnabled(BORNO_KHIPRO) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_JATIYO, Settings::IsLayoutEnabled(LAYOUT_JATIYO) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_PROBHAT, Settings::IsLayoutEnabled(LAYOUT_PROBHAT) ? BST_CHECKED : BST_UNCHECKED);

        CheckDlgButton(hDlg, IDC_CHK_PHONETIC_CORRECTION, Settings::GetPhoneticCorrection() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_TYPING_STYLE, (Settings::GetTypingStyle() == TYPING_STYLE_TRADITIONAL) ? BST_CHECKED : BST_UNCHECKED);
        return (INT_PTR)TRUE;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        return (INT_PTR)GetStockObject(NULL_BRUSH);
    }


    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_CHK_BORNO_PHONETIC ||
            LOWORD(wParam) == IDC_CHK_AVRO_PHONETIC ||
            LOWORD(wParam) == IDC_CHK_KHIPRO ||
            LOWORD(wParam) == IDC_CHK_JATIYO ||
            LOWORD(wParam) == IDC_CHK_PROBHAT)
        {
            if (IsDlgButtonChecked(hDlg, IDC_CHK_BORNO_PHONETIC) == BST_UNCHECKED &&
                IsDlgButtonChecked(hDlg, IDC_CHK_AVRO_PHONETIC) == BST_UNCHECKED &&
                IsDlgButtonChecked(hDlg, IDC_CHK_KHIPRO) == BST_UNCHECKED &&
                IsDlgButtonChecked(hDlg, IDC_CHK_JATIYO) == BST_UNCHECKED &&
                IsDlgButtonChecked(hDlg, IDC_CHK_PROBHAT) == BST_UNCHECKED)
            {
                CheckDlgButton(hDlg, LOWORD(wParam), BST_CHECKED);
            }
        }
        else if (LOWORD(wParam) == IDOK)
        {
            BOOL showBorno = (IsDlgButtonChecked(hDlg, IDC_CHK_BORNO_PHONETIC) == BST_CHECKED);
            BOOL showAvro = (IsDlgButtonChecked(hDlg, IDC_CHK_AVRO_PHONETIC) == BST_CHECKED);
            BOOL showKhipro = (IsDlgButtonChecked(hDlg, IDC_CHK_KHIPRO) == BST_CHECKED);
            BOOL showJatiyo = (IsDlgButtonChecked(hDlg, IDC_CHK_JATIYO) == BST_CHECKED);
            BOOL showProbhat = (IsDlgButtonChecked(hDlg, IDC_CHK_PROBHAT) == BST_CHECKED);

            if (!showBorno && !showAvro && !showKhipro && !showJatiyo && !showProbhat)
            {
                showBorno = TRUE;
                CheckDlgButton(hDlg, IDC_CHK_BORNO_PHONETIC, BST_CHECKED);
            }

            BOOL phoneticCorr = (IsDlgButtonChecked(hDlg, IDC_CHK_PHONETIC_CORRECTION) == BST_CHECKED);
            DWORD typingSt = (IsDlgButtonChecked(hDlg, IDC_CHK_TYPING_STYLE) == BST_CHECKED) ? TYPING_STYLE_TRADITIONAL : TYPING_STYLE_NORMAL;

            Settings::SetLayoutEnabled(BORNO_PHONETIC, showBorno);
            Settings::SetLayoutEnabled(BORNO_AVRO, showAvro);
            Settings::SetLayoutEnabled(BORNO_KHIPRO, showKhipro);
            Settings::SetLayoutEnabled(LAYOUT_JATIYO, showJatiyo);
            Settings::SetLayoutEnabled(LAYOUT_PROBHAT, showProbhat);

            if (!Settings::IsLayoutEnabled(Settings::GetCurrentLayout()))
            {
                if (showBorno) Settings::SetCurrentLayout(BORNO_PHONETIC);
                else if (showAvro) Settings::SetCurrentLayout(BORNO_AVRO);
                else if (showKhipro) Settings::SetCurrentLayout(BORNO_KHIPRO);
                else if (showJatiyo) Settings::SetCurrentLayout(LAYOUT_JATIYO);
                else if (showProbhat) Settings::SetCurrentLayout(LAYOUT_PROBHAT);
            }

            Settings::SetPhoneticCorrection(phoneticCorr);
            Settings::SetTypingStyle(typingSt);

            CRegKey key;
            if (key.Create(HKEY_CURRENT_USER, L"Software\\BornoTSF") == ERROR_SUCCESS) {
                key.SetStringValue(L"EnabledLayouts", Settings::GetEnabledLayouts().c_str());
                key.SetDWORDValue(L"PhoneticCorrection", phoneticCorr ? 1 : 0);
                key.SetDWORDValue(L"TypingStyle", typingSt);
            }

            extern BOOL UpdateProfileRegistration(REFGUID guidProfile, BOOL enable);
            UpdateProfileRegistration(Global::BornoTSFGuidProfile, showBorno);
            UpdateProfileRegistration(Global::GuidProfileAvroPhonetic, showAvro);
            UpdateProfileRegistration(Global::GuidProfileKhipro, showKhipro);
            UpdateProfileRegistration(Global::GuidProfileJatiyo, showJatiyo);
            UpdateProfileRegistration(Global::GuidProfileProbhat, showProbhat);

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDC_BTN_GET_BORNO)
        {
            ShellExecuteW(NULL, L"open", L"https://codepotro.com/borno/", NULL, NULL, SW_SHOWNORMAL);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDC_BTN_ABOUT)
        {
            DialogBoxW(Global::dllInstanceHandle, MAKEINTRESOURCEW(IDD_ABOUT), hDlg, AboutDlgProc);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


HWND CLangBarItemButton::_GetFocusedContextWindow() {
  HWND hwnd = NULL;
  ITfDocumentMgr *pDocMgr;
  if (_pThreadMgr->GetFocus(&pDocMgr) == S_OK && pDocMgr != NULL) {
    ITfContext *pContext;
    if (pDocMgr->GetTop(&pContext) == S_OK && pContext != NULL) {
      ITfContextView *pContextView;
      if (pContext->GetActiveView(&pContextView) == S_OK &&
          pContextView != NULL) {
        pContextView->GetWnd(&hwnd);
        pContextView->Release();
      }
      pContext->Release();
    }
    pDocMgr->Release();
  }

  if (hwnd == NULL) {
    HWND hwndForeground = GetForegroundWindow();
    if (GetWindowThreadProcessId(hwndForeground, NULL) == GetCurrentThreadId())
      hwnd = hwndForeground;
  }

  return hwnd;
}

void BornoTSF::_UpdateLanguageBarOnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus)
{
    BOOL needDisableButtons = FALSE;

    if (!pDocMgrFocus) 
    {
        needDisableButtons = TRUE;
    } 
    else 
    {
        IEnumTfContexts* pEnumContext = nullptr;

        if (FAILED(pDocMgrFocus->EnumContexts(&pEnumContext)) || !pEnumContext) 
        {
            needDisableButtons = TRUE;
        } 
        else 
        {
            ULONG fetched = 0;
            ITfContext* pContext = nullptr;

            if (FAILED(pEnumContext->Next(1, &pContext, &fetched)) || fetched != 1) 
            {
                needDisableButtons = TRUE;
            }

            if (!pContext) 
            {
                needDisableButtons = TRUE;
            } 
            else 
            {
                pContext->Release();
            }
        }

        if (pEnumContext) 
        {
            pEnumContext->Release();
        }
    }

    CCompositionProcessorEngine* pCompositionProcessorEngine = _pCompositionProcessorEngine;
    if (pCompositionProcessorEngine)
    {
        pCompositionProcessorEngine->SetLanguageBarStatus(TF_LBI_STATUS_DISABLED, needDisableButtons);
    }
}

CLangBarItemButton::CLangBarItemButton(REFGUID guidLangBar, LPCWSTR description, LPCWSTR tooltip, DWORD onIconIndex, DWORD offIconIndex, BOOL isSecureMode)
{
    _refCount = 1;
    _pThreadMgr = nullptr;
    _pCompartment = nullptr;
    _pCompartmentEventSink = nullptr;
    _isAddedToLanguageBar = FALSE;
    _status = 0;
    _isSecureMode = isSecureMode;
    _pTooltipText = tooltip;

    DllAddRef();

    _tfLangBarItemInfo.clsidService = Global::BornoTSFCLSID;
    _tfLangBarItemInfo.guidItem = guidLangBar;
    _tfLangBarItemInfo.dwStyle = (TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY);
    _tfLangBarItemInfo.ulSort = 0;
    StringCchCopy(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), description);

    _pLangBarItemSink = nullptr;
    _onIconIndex = onIconIndex;
    _offIconIndex = offIconIndex;
}

CLangBarItemButton::~CLangBarItemButton()
{
    DllRelease();
}

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem))
    {
        *ppvObj = (ITfLangBarItem *)this;
    }
    else if (IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
    return ++_refCount;
}

STDAPI_(ULONG) CLangBarItemButton::Release()
{
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

STDAPI CLangBarItemButton::GetInfo(_Out_ TF_LANGBARITEMINFO *pInfo)
{
    if (pInfo == nullptr)
    {
        return E_INVALIDARG;
    }

    *pInfo = _tfLangBarItemInfo;

    return S_OK;
}

STDAPI CLangBarItemButton::GetStatus(_Out_ DWORD *pdwStatus)
{
    if (pdwStatus == nullptr)
    {
        return E_INVALIDARG;
    }

    *pdwStatus = _status;

    return S_OK;
}

static BOOL IsSystemLightTheme()
{
    DWORD dwLightTheme = 0;
    DWORD dwSize = sizeof(dwLightTheme);
    LONG lResult = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"SystemUsesLightTheme",
        RRF_RT_REG_DWORD,
        NULL,
        &dwLightTheme,
        &dwSize);

    if (lResult == ERROR_SUCCESS)
    {
        return (dwLightTheme != 0);
    }
    return FALSE;
}

static HICON CreateLayoutIcon(DWORD layoutId)
{
    BOOL isLight = IsSystemLightTheme();
    UINT resId = IDI_ICON_BORNO_DARK;

    switch (layoutId)
    {
    case BORNO_PHONETIC:
        resId = isLight ? IDI_ICON_BORNO_LIGHT : IDI_ICON_BORNO_DARK;
        break;
    case BORNO_AVRO:
        resId = isLight ? IDI_ICON_AVRO_LIGHT : IDI_ICON_AVRO_DARK;
        break;
    case BORNO_KHIPRO:
        resId = isLight ? IDI_ICON_KHIPRO_LIGHT : IDI_ICON_KHIPRO_DARK;
        break;
    case LAYOUT_JATIYO:
        resId = isLight ? IDI_ICON_JATIYO_LIGHT : IDI_ICON_JATIYO_DARK;
        break;
    case LAYOUT_PROBHAT:
        resId = isLight ? IDI_ICON_PROBHAT_LIGHT : IDI_ICON_PROBHAT_DARK;
        break;
    default:
        resId = isLight ? IDI_ICON_BORNO_LIGHT : IDI_ICON_BORNO_DARK;
        break;
    }

    int cx = GetSystemMetrics(SM_CXSMICON);
    int cy = GetSystemMetrics(SM_CYSMICON);

    HICON hIcon = (HICON)LoadImage(
        Global::dllInstanceHandle,
        MAKEINTRESOURCE(resId),
        IMAGE_ICON,
        cx > 0 ? cx : 16,
        cy > 0 ? cy : 16,
        LR_DEFAULTCOLOR);

    if (hIcon != NULL)
    {
        return hIcon;
    }

    HDC hdcScreen = GetDC(NULL);
    if (!hdcScreen) return NULL;
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    if (!hdcMem) {
        ReleaseDC(NULL, hdcScreen);
        return NULL;
    }

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = NULL;
    HBITMAP hbmpColor = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!hbmpColor || !pBits) {
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return NULL;
    }

    ZeroMemory(pBits, cx * cy * 4);

    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hbmpColor);

    LPCWSTR label = L"বর্ণ";
    switch (layoutId)
    {
    case BORNO_PHONETIC: label = L"বর্ণ"; break;
    case BORNO_AVRO:     label = L"অভ্র"; break;
    case BORNO_KHIPRO:   label = L"ক্ষিপ্র"; break;
    case LAYOUT_JATIYO:   label = L"জাতীয়"; break;
    case LAYOUT_PROBHAT:  label = L"প্রভাত"; break;
    default:             label = L"বর্ণ"; break;
    }

    COLORREF textColor = isLight ? RGB(32, 32, 32) : RGB(255, 255, 255);

    SetBkMode(hdcMem, TRANSPARENT);
    SetTextColor(hdcMem, textColor);

    int fontHeight = -((cy * 55) / 100);

    HFONT hFont = CreateFontW(
        fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    if (!hFont) {
        hFont = CreateFontW(
            fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Nirmala UI");
    }

    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
    RECT rect = { 0, 0, cx, cy };
    DrawTextW(hdcMem, label, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdcMem, hOldFont);
    DeleteObject(hFont);

    DWORD* pPixels = (DWORD*)pBits;
    for (int i = 0; i < cx * cy; i++) {
        if ((pPixels[i] & 0x00FFFFFF) != 0) {
            pPixels[i] |= 0xFF000000;
        }
    }

    SelectObject(hdcMem, hOldBmp);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    HBITMAP hbmpMask = CreateBitmap(cx, cy, 1, 1, NULL);

    ICONINFO ii = { 0 };
    ii.fIcon = TRUE;
    ii.hbmColor = hbmpColor;
    ii.hbmMask = hbmpMask;

    hIcon = CreateIconIndirect(&ii);

    DeleteObject(hbmpColor);
    DeleteObject(hbmpMask);

    return hIcon;
}

STDAPI CLangBarItemButton::GetIcon(_Out_ HICON *phIcon)
{
    if (phIcon == nullptr)
    {
        return E_INVALIDARG;
    }

    *phIcon = CreateLayoutIcon(Settings::GetCurrentLayout());

    return (*phIcon != nullptr) ? S_OK : E_FAIL;
}

STDAPI CLangBarItemButton::GetText(_Out_ BSTR *pbstrText)
{
    if (pbstrText == nullptr)
    {
        return E_INVALIDARG;
    }

    LPCWSTR text = L"বর্ণ";
    switch (Settings::GetCurrentLayout())
    {
    case BORNO_PHONETIC:
        text = L"বর্ণ";
        break;
    case BORNO_AVRO:
        text = L"অভ্র";
        break;
    case BORNO_KHIPRO:
        text = L"ক্ষিপ্র";
        break;
    case LAYOUT_JATIYO:
        text = L"জাতীয়";
        break;
    case LAYOUT_PROBHAT:
        text = L"প্রভাত";
        break;
    default:
        text = L"বর্ণ";
        break;
    }

    *pbstrText = SysAllocString(text);
    return (*pbstrText == nullptr) ? E_OUTOFMEMORY : S_OK;
}

STDAPI CLangBarItemButton::AdviseSink(__RPC__in REFIID riid, __RPC__in_opt IUnknown *punk, __RPC__out DWORD *pdwCookie)
{
    if (pdwCookie == nullptr)
    {
        return E_INVALIDARG;
    }

    *pdwCookie = 0;

    if (!IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        return CONNECT_E_CANNOTCONNECT;
    }

    if (_pLangBarItemSink != nullptr)
    {
        return CONNECT_E_ADVISELIMIT;
    }

    if (punk == nullptr)
    {
        return E_INVALIDARG;
    }

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = nullptr;
        return E_NOINTERFACE;
    }

    *pdwCookie = _cookie;

    return S_OK;
}

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != _cookie)
    {
        return CONNECT_E_NOCONNECTION;
    }

    if (_pLangBarItemSink == nullptr)
    {
        return CONNECT_E_NOCONNECTION;
    }

    _pLangBarItemSink->Release();
    _pLangBarItemSink = nullptr;

    return S_OK;
}

HRESULT CLangBarItemButton::_AddItem(_In_ ITfThreadMgr *pThreadMgr)
{
    HRESULT hr = S_OK;
    ITfLangBarItemMgr* pLangBarItemMgr = nullptr;

    if (_isAddedToLanguageBar)
    {
        return S_OK;
    }

    if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr)))
    {
        hr = pLangBarItemMgr->AddItem(this);
        if (SUCCEEDED(hr))
        {
            _isAddedToLanguageBar = TRUE;
            _pThreadMgr = pThreadMgr;
            _pThreadMgr->AddRef();
        }

        pLangBarItemMgr->Release();
    }

    return hr;
}

HRESULT CLangBarItemButton::_RemoveItem(_In_ ITfThreadMgr *pThreadMgr)
{
    HRESULT hr = S_OK;
    ITfLangBarItemMgr* pLangBarItemMgr = nullptr;

    if (!_isAddedToLanguageBar)
    {
        return S_OK;
    }

    if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr)))
    {
        hr = pLangBarItemMgr->RemoveItem(this);
        if (SUCCEEDED(hr))
        {
            _isAddedToLanguageBar = FALSE;
            if (_pThreadMgr)
            {
                _pThreadMgr->Release();
                _pThreadMgr = nullptr;
            }
        }

        pLangBarItemMgr->Release();
    }

    return hr;
}

BOOL CLangBarItemButton::_RegisterCompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    _pCompartment = new (std::nothrow) CCompartment(pThreadMgr, tfClientId, guidCompartment);
    if (_pCompartment == nullptr)
    {
        return FALSE;
    }

    _pCompartmentEventSink = new (std::nothrow) CCompartmentEventSink(_CompartmentCallback, this);
    if (_pCompartmentEventSink == nullptr)
    {
        delete _pCompartment;
        _pCompartment = nullptr;
        return FALSE;
    }

    _pCompartmentEventSink->_Advise(pThreadMgr, guidCompartment);

    return TRUE;
}

BOOL CLangBarItemButton::_UnregisterCompartment(_In_ ITfThreadMgr *pThreadMgr)
{
    pThreadMgr;

    if (_pCompartmentEventSink)
    {
        _pCompartmentEventSink->_Unadvise();
        delete _pCompartmentEventSink;
        _pCompartmentEventSink = nullptr;
    }

    if (_pCompartment)
    {
        delete _pCompartment;
        _pCompartment = nullptr;
    }

    return TRUE;
}

void CLangBarItemButton::CleanUp()
{
    if (_pThreadMgr)
    {
        _UnregisterCompartment(_pThreadMgr);
        _RemoveItem(_pThreadMgr);
    }
}

void CLangBarItemButton::SetStatus(DWORD dwStatus, BOOL fSet)
{
    BOOL isChange = FALSE;

    if (fSet)
    {
        if (!(_status & dwStatus))
        {
            _status |= dwStatus;
            isChange = TRUE;
        }
    }
    else
    {
        if (_status & dwStatus)
        {
            _status &= ~dwStatus;
            isChange = TRUE;
        }
    }

    if (isChange && _pLangBarItemSink) 
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
    }
}

void CLangBarItemButton::UpdateIcon()
{
    if (_pLangBarItemSink)
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON | TF_LBI_TEXT);
    }
}

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
    fShow;
    if (_pLangBarItemSink)
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS);
    }
    return S_OK;
}

STDAPI CLangBarItemButton::GetTooltipString(_Out_ BSTR *pbstrToolTip)
{
    *pbstrToolTip = SysAllocString(_pTooltipText);
    return (*pbstrToolTip == nullptr) ? E_OUTOFMEMORY : S_OK;
}

void UpdateLayoutCheckmarks(HMENU hPopupMenu, int currentLayout) {
    auto setCheck = [&](int menuId, int expectedLayout) {
      UINT checkState =
          (currentLayout == expectedLayout) ? MF_CHECKED : MF_UNCHECKED;
      ::CheckMenuItem(hPopupMenu, menuId, MF_BYCOMMAND | checkState);
    };

    setCheck(ID_PHONETICLAYOUT_BORNOPHONETIC, BORNO_PHONETIC);
    setCheck(ID_PHONETICLAYOUT_KHIPRO, BORNO_KHIPRO);
    setCheck(ID_PHONETICLAYOUT_AVROPHONETIC, BORNO_AVRO);
    setCheck(ID_KEYBOARDLAYOUT_PROBHAT, LAYOUT_PROBHAT);
    setCheck(ID_KEYBOARDLAYOUT_JATIYO, LAYOUT_JATIYO);
}

void CLangBarItemButton::loadRefresh() 
{
}

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, _In_ const RECT *prcArea) {
    click; pt; prcArea;

    if (click == TF_LBI_CLK_LEFT || click == TF_LBI_CLK_RIGHT) {
        HWND hwnd = _GetFocusedContextWindow();
        if (hwnd == NULL) {
            return E_FAIL;
        }

        HMENU hMenu = LoadMenuW(Global::dllInstanceHandle, MAKEINTRESOURCEW(IDR_BORNO_TRAY));
        if (hMenu == NULL) {
            return E_FAIL;
        }

        HMENU hPopupMenu = GetSubMenu(hMenu, 0);
        if (hPopupMenu == NULL) {
            DestroyMenu(hMenu);
            return E_FAIL;
        }

        HMENU hLayoutSubMenu = GetSubMenu(hPopupMenu, 0);
        if (hLayoutSubMenu != NULL) {
            if (!Settings::IsLayoutEnabled(BORNO_PHONETIC)) DeleteMenu(hLayoutSubMenu, ID_PHONETICLAYOUT_BORNOPHONETIC, MF_BYCOMMAND);
            if (!Settings::IsLayoutEnabled(BORNO_AVRO)) DeleteMenu(hLayoutSubMenu, ID_PHONETICLAYOUT_AVROPHONETIC, MF_BYCOMMAND);
            if (!Settings::IsLayoutEnabled(BORNO_KHIPRO)) DeleteMenu(hLayoutSubMenu, ID_PHONETICLAYOUT_KHIPRO, MF_BYCOMMAND);
            if (!Settings::IsLayoutEnabled(LAYOUT_JATIYO)) DeleteMenu(hLayoutSubMenu, ID_KEYBOARDLAYOUT_JATIYO, MF_BYCOMMAND);
            if (!Settings::IsLayoutEnabled(LAYOUT_PROBHAT)) DeleteMenu(hLayoutSubMenu, ID_KEYBOARDLAYOUT_PROBHAT, MF_BYCOMMAND);
        }

        UpdateLayoutCheckmarks(hPopupMenu, Settings::GetCurrentLayout());


        UINT selectedId = TrackPopupMenuEx(
            hPopupMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, hwnd, NULL);

        DestroyMenu(hMenu);

        if (selectedId != 0) {
            this->OnMenuSelect(selectedId);
        }
    }

    return S_OK;
}

STDAPI CLangBarItemButton::InitMenu(_In_ ITfMenu *pMenu)
{
    pMenu;
    return S_OK;
}

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
    switch (wID) {
        case ID_PHONETICLAYOUT_KHIPRO: {
            Settings::SetCurrentLayout(BORNO_KHIPRO);
            break;
        }
        case ID_PHONETICLAYOUT_BORNOPHONETIC: {
            Settings::SetCurrentLayout(BORNO_PHONETIC);
            break;
        }
        case ID_PHONETICLAYOUT_AVROPHONETIC: {
            Settings::SetCurrentLayout(BORNO_AVRO);
            break;
        }
        case ID_KEYBOARDLAYOUT_JATIYO: {
            Settings::SetCurrentLayout(LAYOUT_JATIYO);
            break;
        }
        case ID_KEYBOARDLAYOUT_PROBHAT: {
            Settings::SetCurrentLayout(LAYOUT_PROBHAT);
            break;
        }
        case MENU_ITEM_ID_SETTINGS: {
            DialogBoxW(Global::dllInstanceHandle, MAKEINTRESOURCEW(IDD_SETTINGS), GetActiveWindow(), SettingsDlgProc);
            break;
        }
        case MENU_ITEM_ID_KEYMAP: {
               ShellExecuteW(NULL, L"open", L"https://borno.codepotro.com/docs/guide/keyboard-layouts.html?src=BornoTSF", NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        case MENU_ITEM_ID_GET_BORNO: {
            ShellExecuteW(NULL, L"open", L"https://codepotro.com/borno/", NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        case MENU_ITEM_ID_ABOUT: {
            DialogBoxW(Global::dllInstanceHandle, MAKEINTRESOURCEW(IDD_ABOUT), GetActiveWindow(), AboutDlgProc);
            break;
        }
    }
    if (_pLangBarItemSink)
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON | TF_LBI_TEXT);
    }
    return S_OK;
}

HRESULT CLangBarItemButton::_CompartmentCallback(_In_ void *pv, REFGUID guidCompartment)
{
    guidCompartment;
    CLangBarItemButton* pLangBarItemButton = (CLangBarItemButton*)pv;
    if (pLangBarItemButton && pLangBarItemButton->_pLangBarItemSink)
    {
        pLangBarItemButton->_pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON | TF_LBI_TEXT);
    }
    return S_OK;
}
