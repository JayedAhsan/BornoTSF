// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"
#include "Compartment.h"

STDAPI BornoTSF::OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr)
{
    pDocMgr;
    return E_NOTIMPL;
}

STDAPI BornoTSF::OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr)
{
    pDocMgr;
    return E_NOTIMPL;
}

STDAPI BornoTSF::OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus)
{
    pDocMgrPrevFocus;

    _InitTextEditSink(pDocMgrFocus);
    _UpdateLanguageBarOnSetFocus(pDocMgrFocus);

    if (pDocMgrFocus && _pThreadMgr) {
      CCompartment CompartmentKeyboardOpen(_pThreadMgr, _tfClientId,
                                           GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
      CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);
    }

    if (_pDocMgrLastFocused)
    {
        _pDocMgrLastFocused->Release();
        _pDocMgrLastFocused = nullptr;
    }

    _pDocMgrLastFocused = pDocMgrFocus;

    if (_pDocMgrLastFocused)
    {
        _pDocMgrLastFocused->AddRef();
    }

    return S_OK;
}

STDAPI BornoTSF::OnPushContext(_In_ ITfContext *pContext)
{
    pContext;
    return E_NOTIMPL;
}

STDAPI BornoTSF::OnPopContext(_In_ ITfContext *pContext)
{
    pContext;
    return E_NOTIMPL;
}

BOOL BornoTSF::_InitThreadMgrEventSink()
{
    ITfSource* pSource = nullptr;
    BOOL ret = FALSE;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        return ret;
    }

    if (FAILED(pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_threadMgrEventSinkCookie)))
    {
        _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    ret = TRUE;

Exit:
    pSource->Release();
    return ret;
}

void BornoTSF::_UninitThreadMgrEventSink()
{
    ITfSource* pSource = nullptr;

    if (_threadMgrEventSinkCookie == TF_INVALID_COOKIE)
    {
        return; 
    }

    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        pSource->UnadviseSink(_threadMgrEventSinkCookie);
        pSource->Release();
    }

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
