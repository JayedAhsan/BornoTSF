// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

class BornoTSF;

class CEditSessionBase : public ITfEditSession
{
public:
    CEditSessionBase(_In_ BornoTSF *pTextService, _In_ ITfContext *pContext);
    virtual ~CEditSessionBase();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) = 0;

protected:
    ITfContext *_pContext;
    BornoTSF *_pTextService;

private:
    LONG _refCount;
};
