// SPDX-License-Identifier: GPL-3.0-or-later


#include "Private.h"
#include "globals.h"
#include "DisplayAttributeInfo.h"
#include "TfInputProcessorProfile.h"

const WCHAR CDisplayAttributeInfoInput::_s_szValueName[] = L"DisplayAttributeInput";
const WCHAR CDisplayAttributeInfoConverted::_s_szValueName[] = L"DisplayAttributeConverted";


const WCHAR CDisplayAttributeInfoInput::_s_szDescription[] = L"Borno Text Service Display Attribute Input";
const WCHAR CDisplayAttributeInfoConverted::_s_szDescription[] = L"Borno Text Service Display Attribute Converted";

//+---------------------------------------------------------------------------
//
// Tried to mimic the Windows 11 style.... //2025/09 saad a1bf1e27fdfc4b6a62988ace6be963722cb82326
//
//----------------------------------------------------------------------------

const TF_DISPLAYATTRIBUTE CDisplayAttributeInfoInput::_s_DisplayAttribute =
{
	{ TF_CT_COLORREF, RGB(0, 103,206) },
	{ TF_CT_NONE, 0 },
	TF_LS_DOT,
	FALSE,
	{ TF_CT_COLORREF, RGB(0, 103,206) },
	TF_ATTR_INPUT
};

const TF_DISPLAYATTRIBUTE CDisplayAttributeInfoConverted::_s_DisplayAttribute =
{
	{ TF_CT_COLORREF, RGB(255, 255, 255) },
	{ TF_CT_COLORREF, RGB(0, 255, 255) },
	TF_LS_NONE,
	FALSE,
	{ TF_CT_NONE, 0 },
	TF_ATTR_TARGET_CONVERTED
};


CDisplayAttributeInfo::CDisplayAttributeInfo()
{
	DllAddRef();

	_pguid = nullptr;
	_pDisplayAttribute = nullptr;
	_pValueName = nullptr;

	_refCount = 1;
}



CDisplayAttributeInfo::~CDisplayAttributeInfo()
{
	DllRelease();
}


STDAPI CDisplayAttributeInfo::QueryInterface(REFIID riid, _Outptr_ void** ppvObj)
{
	if (ppvObj == nullptr)
		return E_INVALIDARG;

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
	{
		*ppvObj = (ITfDisplayAttributeInfo*)this;
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


ULONG CDisplayAttributeInfo::AddRef(void)
{
	return ++_refCount;
}


ULONG CDisplayAttributeInfo::Release(void)
{
	LONG cr = --_refCount;

	assert(_refCount >= 0);

	if (_refCount == 0)
	{
		delete this;
	}

	return cr;
}


STDAPI CDisplayAttributeInfo::GetGUID(_Out_ GUID* pguid)
{
	if (pguid == nullptr)
		return E_INVALIDARG;

	if (_pguid == nullptr)
		return E_FAIL;

	*pguid = *_pguid;

	return S_OK;
}


STDAPI CDisplayAttributeInfo::GetDescription(_Out_ BSTR* pbstrDesc)
{
	BSTR tempDesc;

	if (pbstrDesc == nullptr)
	{
		return E_INVALIDARG;
	}

	*pbstrDesc = nullptr;

	if ((tempDesc = SysAllocString(_pDescription)) == nullptr)
	{
		return E_OUTOFMEMORY;
	}

	*pbstrDesc = tempDesc;

	return S_OK;
}


STDAPI CDisplayAttributeInfo::GetAttributeInfo(_Out_ TF_DISPLAYATTRIBUTE* ptfDisplayAttr)
{
	if (ptfDisplayAttr == nullptr)
	{
		return E_INVALIDARG;
	}


	*ptfDisplayAttr = *_pDisplayAttribute;

	ZeroMemory(ptfDisplayAttr, sizeof(TF_DISPLAYATTRIBUTE));


	ptfDisplayAttr->crText.type = TF_CT_NONE;
	ptfDisplayAttr->crBk.type = TF_CT_NONE;


	ptfDisplayAttr->crLine.type = TF_CT_COLORREF;
	ptfDisplayAttr->crLine.cr = RGB(0, 0, 0);


	ptfDisplayAttr->lsStyle =
		TF_LS_SQUIGGLE;

	ptfDisplayAttr->fBoldLine =
		FALSE;

	ptfDisplayAttr->bAttr = TF_ATTR_INPUT;

	return S_OK;
}


STDAPI CDisplayAttributeInfo::SetAttributeInfo(_In_ const TF_DISPLAYATTRIBUTE* ptfDisplayAttr)
{
	ptfDisplayAttr;

	return E_NOTIMPL;
}


STDAPI CDisplayAttributeInfo::Reset()
{
	return SetAttributeInfo(_pDisplayAttribute);
}
