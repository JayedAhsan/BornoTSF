// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"

STDAPI BornoTSF::OnEndEdit(__RPC__in_opt ITfContext *pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord *pEditRecord)
{
    BOOL isSelectionChanged;

    if (pEditRecord == nullptr)
    {
        return E_INVALIDARG;
    }
    if (SUCCEEDED(pEditRecord->GetSelectionStatus(&isSelectionChanged)) &&
        isSelectionChanged)
    {
        if (_IsComposing())
        {
            TF_SELECTION tfSelection;
            ULONG fetched = 0;

            if (pContext == nullptr)
            {
                return E_INVALIDARG;
            }
            if (FAILED(pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
            {
                return S_FALSE;
            }

            ITfRange* pRangeComposition = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
            {
                if (!_IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition))
                {
                    _EndComposition(pContext);
                }
                pRangeComposition->Release();
            }

            tfSelection.range->Release();
        }
    }

    return S_OK;
}

BOOL BornoTSF::_InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr)
{
    ITfSource* pSource = nullptr;
    BOOL ret = TRUE;

    if (_textEditSinkCookie != TF_INVALID_COOKIE)
    {
        if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource)))
        {
            pSource->UnadviseSink(_textEditSinkCookie);
            pSource->Release();
        }

        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = nullptr;
        _textEditSinkCookie = TF_INVALID_COOKIE;
    }

    if (pDocMgr == nullptr)
    {
        return TRUE;
    }

    if (FAILED(pDocMgr->GetTop(&_pTextEditSinkContext)))
    {
        return FALSE;
    }

    if (_pTextEditSinkContext == nullptr)
    {
        return TRUE;
    }

    ret = FALSE;
    if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        if (SUCCEEDED(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_textEditSinkCookie)))
        {
            ret = TRUE;
        }
        else
        {
            _textEditSinkCookie = TF_INVALID_COOKIE;
        }
        pSource->Release();
    }

    if (ret == FALSE)
    {
        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = nullptr;
    }

    return ret;
}
