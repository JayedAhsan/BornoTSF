// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "BornoTSF.h"

class CStartCompositionEditSession : public CEditSessionBase
{
public:
    CStartCompositionEditSession(_In_ BornoTSF *pTextService, _In_ ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
    }

    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection* pInsertAtSelection = nullptr;
    ITfRange* pRangeInsert = nullptr;
    ITfContextComposition* pContextComposition = nullptr;
    ITfComposition* pComposition = nullptr;

    if (FAILED(_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection)))
    {
        goto Exit;
    }

    if (FAILED(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert)))
    {
        goto Exit;
    }

    if (FAILED(_pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition)))
    {
        goto Exit;
    }

    if (SUCCEEDED(pContextComposition->StartComposition(ec, pRangeInsert, _pTextService, &pComposition)) && (nullptr != pComposition))
    {
        _pTextService->_SetComposition(pComposition);

        TF_SELECTION tfSelection;
        tfSelection.range = pRangeInsert;
        tfSelection.style.ase = TF_AE_NONE;
        tfSelection.style.fInterimChar = FALSE;

        _pContext->SetSelection(ec, 1, &tfSelection);
        _pTextService->_SaveCompositionContext(_pContext);
    }

Exit:
    if (nullptr != pContextComposition)
    {
        pContextComposition->Release();
    }

    if (nullptr != pRangeInsert)
    {
        pRangeInsert->Release();
    }

    if (nullptr != pInsertAtSelection)
    {
        pInsertAtSelection->Release();
    }

    return S_OK;
}

void BornoTSF::_StartComposition(_In_ ITfContext *pContext)
{
    CStartCompositionEditSession* pStartCompositionEditSession = new (std::nothrow) CStartCompositionEditSession(this, pContext);

    if (nullptr != pStartCompositionEditSession)
    {
        HRESULT hr = S_OK;
        pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
        pStartCompositionEditSession->Release();
    }
}

void BornoTSF::_SaveCompositionContext(_In_ ITfContext *pContext)
{
    assert(_pContext == nullptr);
    pContext->AddRef();
    _pContext = pContext;
}
