// SPDX-License-Identifier: GPL-3.0-or-later

#include "Globals.h"
#include "BornoTSFBaseStructure.h"

//---------------------------------------------------------------------
//
// CLSIDToString
//
//---------------------------------------------------------------------

const BYTE GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
};

const WCHAR HexDigits[] = L"0123456789ABCDEF";

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR *pCLSIDString)
{
    WCHAR* pTemp = pCLSIDString;
    const BYTE* pBytes = (const BYTE *) &refGUID;

    DWORD j = 0;
    pTemp[j++] = L'{';
    for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++)
    {
        if (GuidSymbols[i] == '-')
        {
            pTemp[j++] = L'-';
        }
        else
        {
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0xF0) >> 4 ];
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0x0F) ];
        }
    }

    pTemp[j++] = L'}';
    pTemp[j] = L'\0';

    return TRUE;
}

HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && (*pwszBuffer != wch) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

CStringRange::CStringRange()
{
    _stringBufLen = 0;
    _pStringBuf = nullptr;
}

CStringRange::~CStringRange() 
{ 
}

const WCHAR *CStringRange::Get() const
{
    return _pStringBuf;
}

const DWORD_PTR CStringRange::GetLength() const
{
    return _stringBufLen;
}

void CStringRange::Clear()
{
    _stringBufLen = 0;
    _pStringBuf = nullptr;
}

void CStringRange::Set(const WCHAR *pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = pwch;
}

void CStringRange::Set(CStringRange &sr)
{
    *this = sr;
}

CStringRange& CStringRange::operator =(const CStringRange& sr)
{
    _stringBufLen = sr._stringBufLen;
    _pStringBuf = sr._pStringBuf;
    return *this;
}

void CStringRange::CharNext(_Inout_ CStringRange* pCharNext)
{
    if (!_stringBufLen)
    {
        pCharNext->_stringBufLen = 0;
        pCharNext->_pStringBuf = nullptr;
    }
    else
    {
        pCharNext->_stringBufLen = _stringBufLen;
        pCharNext->_pStringBuf = _pStringBuf;

        while (pCharNext->_stringBufLen)
        {
            BOOL isSurrogate = (IS_HIGH_SURROGATE(*pCharNext->_pStringBuf) || IS_LOW_SURROGATE(*pCharNext->_pStringBuf));
            pCharNext->_stringBufLen--;
            pCharNext->_pStringBuf++;
            if (!isSurrogate)
            {
                break;
            }
        }
    }
}

int CStringRange::Compare(LCID locale, _In_ CStringRange* pString1, _In_ CStringRange* pString2)
{
    return CompareString(locale, 
        NORM_IGNORECASE, 
        pString1->Get(), 
        (DWORD)pString1->GetLength(), 
        pString2->Get(), 
        (DWORD)pString2->GetLength());
}

CPunctuationPair::CPunctuationPair()
{
    _punctuation._Code = 0;
    _punctuation._Punctuation = 0;
    _pairPunctuation = 0;
    _isPairToggle = FALSE;
}

CPunctuationPair::CPunctuationPair(WCHAR code, WCHAR punctuation, WCHAR pair)
{
    _punctuation._Code = code;
    _punctuation._Punctuation = punctuation;
    _pairPunctuation = pair;
    _isPairToggle = FALSE;
}

