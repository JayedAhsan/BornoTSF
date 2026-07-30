// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"

void BornoTSF::_ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext)
{
    ITfRange* pRangeComposition = nullptr;
    ITfProperty* pDisplayAttributeProperty = nullptr;

    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        return;
    }

    if (SUCCEEDED(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
    {
        pDisplayAttributeProperty->Clear(ec, pRangeComposition);
        pDisplayAttributeProperty->Release();
    }

    pRangeComposition->Release();
}

BOOL BornoTSF::_SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute)
{
    ITfRange* pRangeComposition = nullptr;
    ITfProperty* pDisplayAttributeProperty = nullptr;
    HRESULT hr = S_OK;

    hr = _pComposition->GetRange(&pRangeComposition);
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = E_FAIL;

    if (SUCCEEDED(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = gaDisplayAttribute; 

        hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);
        pDisplayAttributeProperty->Release();
    }

    pRangeComposition->Release();
    return (hr == S_OK);
}

BOOL BornoTSF::_InitDisplayAttributeGuidAtom()
{
    ITfCategoryMgr* pCategoryMgr = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = pCategoryMgr->RegisterGUID(Global::BornoTSFGuidDisplayAttributeInput, &_gaDisplayAttributeInput);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = pCategoryMgr->RegisterGUID(Global::BornoTSFGuidDisplayAttributeConverted, &_gaDisplayAttributeConverted);
    if (FAILED(hr))
    {
        goto Exit;
    }

Exit:
    pCategoryMgr->Release();

    return (hr == S_OK);
}
