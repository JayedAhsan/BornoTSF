// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "EditSession.h"
#include "Globals.h"

class CKeyHandlerEditSession : public CEditSessionBase
{
public:
    CKeyHandlerEditSession(BornoTSF *pTextService, ITfContext *pContext, UINT uCode, WCHAR wch, _KEYSTROKE_STATE keyState) : CEditSessionBase(pTextService, pContext)
    {
        _uCode = uCode;
        _wch = wch;
        _KeyState = keyState;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    UINT _uCode;
    WCHAR _wch;
    _KEYSTROKE_STATE _KeyState;
};
