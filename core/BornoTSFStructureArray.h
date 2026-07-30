// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>
#include <assert.h>

template<class T>
class CBornoTSFStructureArray
{
    typedef typename    std::vector<T>         value_type;
    typedef const       T&                     CONST_REF;
    typedef typename    value_type             CBornoTSFArray;
    typedef typename    value_type::iterator   CBornoTSFIter;

public:
    CBornoTSFStructureArray(): _imeVector()
    {
    }

    explicit CBornoTSFStructureArray(size_t iCount): _imeVector(iCount)
    {
    }

    CBornoTSFStructureArray(size_t iCount, CONST_REF tVal): _imeVector(iCount, tVal)
    {
    }

    virtual ~CBornoTSFStructureArray() {}

    inline CONST_REF GetAt(size_t iIndex) const
    {
        assert(iIndex <= _imeVector.size());
        assert(_imeVector.size() > 0);

        return _imeVector[iIndex];
    }

    inline T& GetAt(size_t iIndex)
    {
        assert(iIndex <= _imeVector.size());
        assert(_imeVector.size() > 0);

        return _imeVector[iIndex];
    }

    void RemoveAt(size_t iIndex, size_t iElements)
    {
        assert(iIndex <= _imeVector.size());
        assert(_imeVector.size() > 0);

        CBornoTSFIter beginIter = _imeVector.begin() + iIndex;
        CBornoTSFIter lastIter = beginIter + iElements - 1;

        _imeVector.erase(beginIter, lastIter);
    }

    size_t Count() const { return _imeVector.size(); }

    void Append(const T& tVal)
    {
        _imeVector.push_back(tVal);
    }

    void Clear()
    {
        _imeVector.clear();
    }

private:
    CBornoTSFArray _imeVector;   // the actual array of data
    CBornoTSFIter  _imeIter;
};
