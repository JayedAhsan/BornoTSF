// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "StringRange.h"
#include "PointerArray.h"

class CDisplayString
{
public:
    CDisplayString() { }
    ~CDisplayString() { }

    int Count() { }
   VOID SetLogicalFont(LOGFONTW LogFont) { }
    VOID GetLogicalFont(LOGFONTW* pLogFont) { }

private:
    LOGFONTW                       _logfont;
};
