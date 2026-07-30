// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once


class CEnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo
{
public:
	CEnumDisplayAttributeInfo();
	~CEnumDisplayAttributeInfo();


	STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);


	STDMETHODIMP Clone(_Out_ IEnumTfDisplayAttributeInfo** ppEnum);
	STDMETHODIMP Next(ULONG ulCount, __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo** rgInfo, __RPC__out ULONG* pcFetched);
	STDMETHODIMP Reset();
	STDMETHODIMP Skip(ULONG ulCount);

private:
	LONG _index;
	LONG _refCount;
};