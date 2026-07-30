// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"
#include "DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

STDAPI BornoTSF::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo* pAttributeEnum = nullptr;

    if (ppEnum == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppEnum = nullptr;

    pAttributeEnum = new (std::nothrow) CEnumDisplayAttributeInfo();
    if (pAttributeEnum == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    *ppEnum = pAttributeEnum;

    return S_OK;
}

STDAPI BornoTSF::GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo **ppInfo)
{
    if (ppInfo == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppInfo = nullptr;

    if (IsEqualGUID(guidInfo, Global::BornoTSFGuidDisplayAttributeInput))
    {
        *ppInfo = new (std::nothrow) CDisplayAttributeInfoInput();
        if ((*ppInfo) == nullptr)
        {
            return E_OUTOFMEMORY;
        }
    }
    else if (IsEqualGUID(guidInfo, Global::BornoTSFGuidDisplayAttributeConverted))
    {
        *ppInfo = new (std::nothrow) CDisplayAttributeInfoConverted();
        if ((*ppInfo) == nullptr)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}
