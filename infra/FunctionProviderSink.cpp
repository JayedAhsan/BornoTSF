// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "BornoTSF.h"

BOOL BornoTSF::_InitFunctionProviderSink()
{
    ITfSourceSingle* pSourceSingle = nullptr;
    BOOL ret = FALSE;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle)))
    {
        IUnknown* punk = nullptr;
        if (SUCCEEDED(QueryInterface(IID_IUnknown, (void **)&punk)))
        {
            if (SUCCEEDED(pSourceSingle->AdviseSingleSink(_tfClientId, IID_ITfFunctionProvider, punk)))
            {
                ret = TRUE;
            }
            punk->Release();
        }
        pSourceSingle->Release();
    }
    return ret;
}

void BornoTSF::_UninitFunctionProviderSink()
{
    ITfSourceSingle* pSourceSingle = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle)))
    {
        pSourceSingle->UnadviseSingleSink(_tfClientId, IID_ITfFunctionProvider);
        pSourceSingle->Release();
    }
}
