// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "BornoTSF.h"
#include "CompositionProcessorEngine.h"

BOOL BornoTSF::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
{
    LONG lResult = 0;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) 
        || (lResult > 0))
    {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) 
        || (lResult < 0))
    {
        return FALSE;
    }

    return TRUE;
}

VOID BornoTSF::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
{
    isForce; pContext;
    if (_pCompositionProcessorEngine) {
        _pCompositionProcessorEngine->PurgeVirtualKey();
    }
}

HRESULT BornoTSF::_HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _DeleteCandidateList(FALSE, pContext);
    _TerminateComposition(ec, pContext);
    return S_OK;
}

HRESULT BornoTSF::_HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _RemoveDummyCompositionForComposing(ec, _pComposition);
    _DeleteCandidateList(FALSE, pContext);
    _TerminateComposition(ec, pContext);
    return S_OK;
}

HRESULT BornoTSF::_HandleCompositionInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    CCompositionProcessorEngine* pCompositionProcessorEngine = _pCompositionProcessorEngine;

    if (!_IsComposing())
    {
        _StartComposition(pContext);
    }

    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
    {
        return S_FALSE;
    }

    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);
        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    pCompositionProcessorEngine->AddVirtualKey(wch);
    _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);

Exit:
    tfSelection.range->Release();
    return S_OK;
}

HRESULT BornoTSF::_HandleCompositionInputWorker(_In_ CCompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;
    BornoTSFArray<CStringRange> readingStrings;

    pCompositionProcessorEngine->GetReadingStrings(&readingStrings);

    for (UINT index = 0; index < readingStrings.Count(); index++)
    {
        hr = _AddComposingAndChar(ec, pContext, readingStrings.GetAt(index));
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

HRESULT BornoTSF::_HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCandidateList)
{
    isCandidateList;
    if (_IsComposing())
    {
        ULONG fetched = 0;
        TF_SELECTION tfSelection;

        if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
        {
            return S_FALSE;
        }

        ITfRange* pRangeComposition = nullptr;
        if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
        {
            if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
            {
                _EndComposition(pContext);
            }
            pRangeComposition->Release();
        }

        tfSelection.range->Release();
    }

    _HandleCancel(ec, pContext);
    return S_OK;
}

HRESULT BornoTSF::_HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext *pContext)
{
    return _HandleCompositionFinalize(ec, pContext, FALSE);
}

HRESULT BornoTSF::_HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    if (!_IsComposing())
    {
        return S_OK;
    }

    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
    {
        return S_FALSE;
    }

    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);
        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    CCompositionProcessorEngine* pCompositionProcessorEngine = _pCompositionProcessorEngine;
    DWORD_PTR vKeyLen = pCompositionProcessorEngine->GetVirtualKeyLength();

    if (vKeyLen)
    {
        pCompositionProcessorEngine->RemoveVirtualKey(vKeyLen - 1);

        if (pCompositionProcessorEngine->GetVirtualKeyLength())
        {
            _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);
        }
        else
        {
            _HandleCancel(ec, pContext);
        }
    }

Exit:
    tfSelection.range->Release();
    return S_OK;
}

HRESULT BornoTSF::_HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction)
{
    keyFunction;
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;

    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
    {
        return S_OK;
    }

    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        goto Exit;
    }

    pContext->SetSelection(ec, 1, &tfSelection);
    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK;
}

HRESULT BornoTSF::_HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;
    CCompositionProcessorEngine* pCompositionProcessorEngine = _pCompositionProcessorEngine;

    WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(wch);
    CStringRange punctuationString;
    punctuationString.Set(&punctuation, 1);

    hr = _AddCharAndFinalize(ec, pContext, &punctuationString);
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);
    return S_OK;
}

HRESULT BornoTSF::_HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;
    WCHAR fullWidth = Global::FullWidthCharTable[wch - 0x20];
    CStringRange fullWidthString;
    fullWidthString.Set(&fullWidth, 1);

    hr = _AddCharAndFinalize(ec, pContext, &fullWidthString);
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);
    return S_OK;
}

HRESULT BornoTSF::_InvokeKeyHandler(_In_ ITfContext *pContext, UINT code, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState)
{
    flags;
    CKeyHandlerEditSession* pEditSession = nullptr;
    HRESULT hr = E_FAIL;

    pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, code, wch, keyState);
    if (pEditSession == nullptr)
    {
        goto Exit;
    }

    hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
    pEditSession->Release();

Exit:
    return hr;
}
