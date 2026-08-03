#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "BornoTSF.h"
#include "CompositionProcessorEngine.h"
#include "../display/CandidateWindow.h"

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
    CCandidateWindow::GetInstance().ClearCandidates();
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

    std::vector<std::wstring> candidates;
    for (UINT index = 0; index < readingStrings.Count(); index++)
    {
        CStringRange* pStrRange = readingStrings.GetAt(index);
        if (pStrRange && pStrRange->Get() && pStrRange->GetLength() > 0)
        {
            candidates.push_back(std::wstring(pStrRange->Get(), pStrRange->GetLength()));
        }

        hr = _AddComposingAndChar(ec, pContext, readingStrings.GetAt(index));
        if (FAILED(hr))
        {
            return hr;
        }
    }

    bool isPhonetic = (Global::currentLayout == BORNO_PHONETIC || Global::currentLayout == BORNO_AVRO);

    if (pCompositionProcessorEngine->GetVirtualKeyLength() > 0 && isPhonetic)
    {
        std::wstring rawKeys(pCompositionProcessorEngine->GetVirtualKeys(), pCompositionProcessorEngine->GetVirtualKeyLength());

        if (!candidates.empty())
        {
            // direct output
            std::wstring normalParsed = pCompositionProcessorEngine->ApplyRulesWithoutAutoCorrect(rawKeys);

            bool autoCorrectApplied = (!normalParsed.empty() && candidates[0] != normalParsed);

            if (autoCorrectApplied)
            {
                // extended
                if (!normalParsed.empty()) candidates.push_back(normalParsed);
                if (!rawKeys.empty() && rawKeys != normalParsed && rawKeys != candidates[0])
                    candidates.push_back(rawKeys);
            }
            else
            {
                // typed
                if (!rawKeys.empty() && rawKeys != candidates[0])
                    candidates.push_back(rawKeys);
            }
        }
    }

    if (!candidates.empty() && isPhonetic && candidates.size() > 1)
    {
        CCandidateWindow::GetInstance().SetCandidates(candidates);
        CCandidateWindow::GetInstance().MoveToComposition(ec, pContext, _pComposition);
    }
    else
    {
        CCandidateWindow::GetInstance().ClearCandidates();
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

HRESULT BornoTSF::_HandleCandidateSelect(TfEditCookie ec, _In_ ITfContext *pContext, UINT code)
{
    // index
    int index = (int)(code - '1');
    std::wstring selectedStr;

    if (index >= 0 && index < (int)CCandidateWindow::GetInstance().GetCandidateCount())
    {
        selectedStr = CCandidateWindow::GetInstance().GetCandidate((size_t)index);
    }

    if (selectedStr.empty())
    {
        // as it is
        _HandleComplete(ec, pContext);
        return S_OK;
    }

    // selected
    if (_pComposition != nullptr)
    {
        ITfRange* pCompRange = nullptr;
        if (SUCCEEDED(_pComposition->GetRange(&pCompRange)) && pCompRange)
        {
            pCompRange->SetText(ec, 0, selectedStr.c_str(), (LONG)selectedStr.length());
            // mov cursor to end
            pCompRange->Collapse(ec, TF_ANCHOR_END);
            TF_SELECTION sel;
            sel.range = pCompRange;
            sel.style.ase = TF_AE_NONE;
            sel.style.fInterimChar = FALSE;
            pContext->SetSelection(ec, 1, &sel);
            pCompRange->Release();
        }
    }

    // finish
    if (_pCompositionProcessorEngine)
    {
        _pCompositionProcessorEngine->PurgeVirtualKey();
    }

    // clear
    CCandidateWindow::GetInstance().ClearCandidates();
    _TerminateComposition(ec, pContext);

    // forward CandidateCommitKey
    if (Global::CandidateCommitKey != 0)
    {
        UINT vk = Global::CandidateCommitKey;
        Global::CandidateCommitKey = 0;

        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = (WORD)vk;
        inputs[0].ki.dwFlags = 0;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = (WORD)vk;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(2, inputs, sizeof(INPUT));
    }

    return S_OK;
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
