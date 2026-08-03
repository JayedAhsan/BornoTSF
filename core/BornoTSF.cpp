// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"
#include "Compartment.h"
#include <map>
#include <shlobj.h>
#include <sstream>
#include <tchar.h>
#include "CompositionProcessorEngine.h"
#include "../display/CandidateWindow.h"

/* static */
HRESULT BornoTSF::CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj)
{
    BornoTSF* pBornoTSF = nullptr;
    HRESULT hr = S_OK;

    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (nullptr != pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    pBornoTSF = new (std::nothrow) BornoTSF();
    if (pBornoTSF == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    hr = pBornoTSF->QueryInterface(riid, ppvObj);
    pBornoTSF->Release();

    return hr;
}

LONG g_dllRefCount = 0;

BornoTSF::BornoTSF()
{
    DllAddRef();

    _pThreadMgr = nullptr;
    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
    _pTextEditSinkContext = nullptr;
    _textEditSinkCookie = TF_INVALID_COOKIE;
    _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

    _pComposition = nullptr;
    _pCompositionProcessorEngine = nullptr;

    _pDocMgrLastFocused = nullptr;
    _pContext = nullptr;
    _refCount = 1;
}

BornoTSF::~BornoTSF()
{
    DllRelease();
}

STDAPI BornoTSF::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
    {
        *ppvObj = (ITfTextInputProcessorEx *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
    {
        *ppvObj = (ITfKeyEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfActiveLanguageProfileNotifySink))
    {
        *ppvObj = (ITfActiveLanguageProfileNotifySink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCompositionSink))
    {
        *ppvObj = (ITfCompositionSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
    {
        *ppvObj = (ITfDisplayAttributeProvider *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
    {
        *ppvObj = (ITfThreadFocusSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfFunctionProvider))
    {
        *ppvObj = (ITfFunctionProvider *)this;
    }
    else if (IsEqualIID(riid, IID_ITfFunction))
    {
        *ppvObj = (ITfFunction *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) BornoTSF::AddRef()
{
    return ++_refCount;
}

STDAPI_(ULONG) BornoTSF::Release()
{
    LONG cr = --_refCount;
    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

#include "RegKey.h"

void BornoTSF::_LoadSettings() {
    CRegKey key;
    if (key.Open(HKEY_CURRENT_USER, L"Software\\BornoTSF", KEY_READ) == ERROR_SUCCESS) {
        DWORD dwVal = 0;
        if (key.QueryDWORDValue(L"PhoneticCorrection", dwVal) == ERROR_SUCCESS) {
            Global::phoneticCorrection = (dwVal != 0);
        }
        if (key.QueryDWORDValue(L"TypingStyle", dwVal) == ERROR_SUCCESS) {
            Global::typingStyle = dwVal;
        }
        WCHAR szLayouts[512] = { 0 };
        ULONG cchLayouts = ARRAYSIZE(szLayouts);
        if (key.QueryStringValue(L"EnabledLayouts", szLayouts, &cchLayouts) == ERROR_SUCCESS && cchLayouts > 0) {
            Global::enabledLayouts = szLayouts;
        }
    } else {
        Global::phoneticCorrection = TRUE;
        Global::typingStyle = TYPING_STYLE_NORMAL;
        Global::enabledLayouts = L"1,2,3,5,6";
    }
}



STDAPI BornoTSF::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags)
{
    _LoadSettings();
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;
    _dwActivateFlags = dwFlags;

    if (!_InitThreadMgrEventSink())
    {
        goto ExitError;
    }

    ITfDocumentMgr* pDocMgrFocus = nullptr;
    if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocMgrFocus)) && (pDocMgrFocus != nullptr))
    {
        _InitTextEditSink(pDocMgrFocus);
        pDocMgrFocus->Release();
    }

    if (!_InitKeyEventSink())
    {
        goto ExitError;
    }

    if (!_InitActiveLanguageProfileNotifySink())
    {
        goto ExitError;
    }

    if (!_InitThreadFocusSink())
    {
        goto ExitError;
    }

    if (!_InitDisplayAttributeGuidAtom())
    {
        goto ExitError;
    }

    if (!_InitFunctionProviderSink())
    {
        goto ExitError;
    }

    if (!_AddTextProcessorEngine())
    {
        goto ExitError;
    }

    CCandidateWindow::GetInstance().Create();

    return S_OK;

ExitError:
    Deactivate();
    return E_FAIL;
}

STDAPI BornoTSF::Deactivate()
{
    CCandidateWindow::GetInstance().Destroy();

    if (_pCompositionProcessorEngine)
    {
        delete _pCompositionProcessorEngine;
        _pCompositionProcessorEngine = nullptr;
    }

    ITfContext* pContext = _pContext;
    if (_pContext)
    {   
        pContext->AddRef();
        _EndComposition(_pContext);
        pContext->Release();
    }

    _UninitFunctionProviderSink();
    _UninitThreadFocusSink();
    _UninitActiveLanguageProfileNotifySink();
    _UninitKeyEventSink();
    _UninitThreadMgrEventSink();

    CCompartment CompartmentKeyboardOpen(_pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CompartmentKeyboardOpen._ClearCompartment();

    if (_pThreadMgr != nullptr)
    {
        _pThreadMgr->Release();
    }

    _tfClientId = TF_CLIENTID_NULL;

    if (_pDocMgrLastFocused)
    {
        _pDocMgrLastFocused->Release();
        _pDocMgrLastFocused = nullptr;
    }

    return S_OK;
}

HRESULT BornoTSF::GetType(__RPC__out GUID *pguid)
{
    HRESULT hr = E_INVALIDARG;
    if (pguid)
    {
        *pguid = Global::BornoTSFCLSID;
        hr = S_OK;
    }
    return hr;
}

HRESULT BornoTSF::GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc)
{
    HRESULT hr = E_INVALIDARG;
    if (pbstrDesc != nullptr)
    {
        *pbstrDesc = nullptr;
        hr = E_NOTIMPL;
    }
    return hr;
}

HRESULT BornoTSF::GetFunction(__RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk)
{
    HRESULT hr = E_NOINTERFACE;

    if (IsEqualGUID(rguid, GUID_NULL))
    {
        hr = QueryInterface(riid, (void **)ppunk);
    }

    return hr;
}

HRESULT BornoTSF::GetDisplayName(_Out_ BSTR *pbstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;
    if (pbstrDisplayName != nullptr)
    {
        *pbstrDisplayName = nullptr;
        hr = E_NOTIMPL;
    }
    return hr;
}

HRESULT BornoTSF::GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId)
{
    HRESULT hr = E_INVALIDARG;
    if ((ptkblayoutType != nullptr) && (pwPreferredLayoutId != nullptr))
    {
        *ptkblayoutType = TKBLT_OPTIMIZED;
        *pwPreferredLayoutId = TKBL_OPT_SIMPLIFIED_CHINESE_PINYIN;
        hr = S_OK;
    }
    return hr;
}
