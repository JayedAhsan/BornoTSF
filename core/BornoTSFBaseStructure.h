// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "stdafx.h"
#include <vector>
#include "assert.h"

//---------------------------------------------------------------------
// enum
//---------------------------------------------------------------------
enum KEYSTROKE_CATEGORY
{
    CATEGORY_NONE = 0,
    CATEGORY_COMPOSING,

    CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION
};

enum KEYSTROKE_FUNCTION
{
    FUNCTION_NONE = 0,
    FUNCTION_INPUT,

    FUNCTION_CANCEL,
    FUNCTION_FINALIZE_TEXTSTORE,
    FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT,
    FUNCTION_CONVERT,

    FUNCTION_BACKSPACE,
    FUNCTION_MOVE_LEFT,
    FUNCTION_MOVE_RIGHT,

    // Function Double/Single byte
    FUNCTION_DOUBLE_SINGLE_BYTE,

    // Function Punctuation
    FUNCTION_PUNCTUATION,

    // Function Candidate Selection
    FUNCTION_CANDIDATE_SELECT
};

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    KEYSTROKE_CATEGORY Category;
    KEYSTROKE_FUNCTION Function;
};

struct _PUNCTUATION
{
    WCHAR _Code;
    WCHAR _Punctuation;
};

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_ (39) WCHAR *pCLSIDString);

HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);

template<class T>
class BornoTSFArray
{
    typedef typename std::vector<T> BornoTSFInnerArray;
    typedef typename std::vector<T>::iterator BornoTSFInnerIter;

public:
    BornoTSFArray(): _innerVect()
    {
    }

    explicit BornoTSFArray(size_t count): _innerVect(count)
    {
    }

    virtual ~BornoTSFArray()
    {
    }

    inline T* GetAt(size_t index)
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        T& curT = _innerVect.at(index);

        return &(curT);
    }

    inline const T* GetAt(size_t index) const
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        T& curT = _innerVect.at(index);

        return &(curT);
    }

    void RemoveAt(size_t index)
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        BornoTSFInnerIter iter = _innerVect.begin();
        _innerVect.erase(iter + index);
    }

    UINT Count() const 
    { 
        return static_cast<UINT>(_innerVect.size());
    }

    T* Append()
    {
        T newT;
        _innerVect.push_back(newT);
        T& backT = _innerVect.back();

        return &(backT);
    }

    void reserve(size_t Count)
    {
        _innerVect.reserve(Count);
    }

    void Clear()
    {
        _innerVect.clear();
    }

private:
    BornoTSFInnerArray _innerVect;
};


class CStringRange
{
public:
    CStringRange();
    ~CStringRange();

    const WCHAR *Get() const;
    const DWORD_PTR GetLength() const;
    void Clear();
    void Set(const WCHAR *pwch, DWORD_PTR dwLength);
    void Set(CStringRange &sr);
    CStringRange& operator=(const CStringRange& sr);
    void CharNext(_Inout_ CStringRange* pCharNext);
    static int Compare(LCID locale, _In_ CStringRange* pString1, _In_ CStringRange* pString2);

protected:
    DWORD_PTR _stringBufLen;         // Length is in character count.
    const WCHAR *_pStringBuf;    // Buffer which is not add zero terminate.
};

class CPunctuationPair
{
public:
    CPunctuationPair();
    CPunctuationPair(WCHAR code, WCHAR punctuation, WCHAR pair);

    struct _PUNCTUATION _punctuation;
    WCHAR _pairPunctuation;
    BOOL _isPairToggle;
};

