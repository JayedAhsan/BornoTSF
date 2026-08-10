// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"

#include "Settings.h"
#include "KeyHandlerEditSession.h"
#include "Compartment.h"
#include "CompositionProcessorEngine.h"
#include "../display/CandidateWindow.h"

#define THIRDPARTY_NEXTPAGE  static_cast<WORD>(0xF003)
#define THIRDPARTY_PREVPAGE  static_cast<WORD>(0xF004)

__inline UINT VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch)
{
    UINT vkRet = vk;
    if (LOWORD(vk) == VK_PACKET)
    {
        if (wch == L' ')
        {
            vkRet = VK_SPACE;
        }
        else if ((wch >= L'0') && (wch <= L'9'))
        {
            vkRet = static_cast<UINT>(wch);
        }
        else if ((wch >= L'a') && (wch <= L'z'))
        {
            vkRet = (UINT)(L'A') + ((UINT)(L'z') - static_cast<UINT>(wch));
        }
        else if ((wch >= L'A') && (wch <= L'Z'))
        {
            vkRet = static_cast<UINT>(wch);
        }
        else if (wch == THIRDPARTY_NEXTPAGE)
        {
            vkRet = VK_NEXT;
        }
        else if (wch == THIRDPARTY_PREVPAGE)
        {
            vkRet = VK_PRIOR;
        }
    }
    return vkRet;
}

BOOL BornoTSF::_IsKeyEaten(_In_ ITfContext *pContext, UINT codeIn, _Out_ UINT *pCodeOut, _Out_writes_(1) WCHAR *pwch, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    pContext;
    BOOL isOpen = Global::isImeEnabled;

    if (pKeyState)
    {
        pKeyState->Category = CATEGORY_NONE;
        pKeyState->Function = FUNCTION_NONE;
    }
    if (pwch)
    {
        *pwch = L'\0';
    }

    if (_IsKeyboardDisabled())
    {
        return FALSE;
    }

    const BOOL isCtrlDown = (GetKeyState(VK_CONTROL) & 0x8000) ||
                            (GetKeyState(VK_LCONTROL) & 0x8000) ||
                            (GetKeyState(VK_RCONTROL) & 0x8000);

    // Ctrl+Backspace deletes the active composition as one word. Outside a
    // composition it is left to the application, along with other Ctrl keys.
    if (isCtrlDown)
    {
        if (isOpen && codeIn == VK_BACK && _IsComposing() && pKeyState)
        {
            *pCodeOut = VK_BACK;
            pKeyState->Category = CATEGORY_COMPOSING;
            pKeyState->Function = FUNCTION_CANCEL;
            return TRUE;
        }

        return FALSE;
    }

    if (!isOpen)
    {
        return FALSE;
    }

    WCHAR wch = ConvertVKey(codeIn);
    *pCodeOut = VKeyFromVKPacketAndWchar(codeIn, wch);

    if (pwch)
    {
        *pwch = wch;
    }

    CCompositionProcessorEngine *pCompositionProcessorEngine = _pCompositionProcessorEngine;

    if (isOpen && pCompositionProcessorEngine)
    {
        if (pCompositionProcessorEngine->IsVirtualKeyNeed(*pCodeOut, pwch, _IsComposing(), pKeyState))
        {
            return TRUE;
        }
    }

    if (pCompositionProcessorEngine && pCompositionProcessorEngine->IsPunctuation(wch))
    {
        if (pKeyState)
        {
            pKeyState->Category = CATEGORY_COMPOSING;
            pKeyState->Function = FUNCTION_PUNCTUATION;
        }
        return TRUE;
    }

    return FALSE;
}

WCHAR BornoTSF::ConvertVKey(UINT code)
{
    UINT scanCode = MapVirtualKey(code, 0);

    ::BYTE abKbdState[256] = {'\0'};
    if (!GetKeyboardState(abKbdState))
    {
        return 0;
    }

    static HKL hklUS = LoadKeyboardLayout(L"00000409", KLF_NOTELLSHELL);
    HKL hklToUse = hklUS ? hklUS : GetKeyboardLayout(0);

    WCHAR wch = '\0';
    if (ToUnicodeEx(code, scanCode, abKbdState, &wch, 1, 0, hklToUse) == 1)
    {
        return wch;
    }

    return 0;
}

BOOL BornoTSF::_IsKeyboardDisabled()
{
    ITfDocumentMgr* pDocMgrFocus = nullptr;
    ITfContext* pContext = nullptr;
    BOOL isDisabled = FALSE;

    if (_pThreadMgr != nullptr && SUCCEEDED(_pThreadMgr->GetFocus(&pDocMgrFocus)) && pDocMgrFocus != nullptr)
    {
        CCompartment CompartmentKeyboardDisabled(_pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_DISABLED);
        CompartmentKeyboardDisabled._GetCompartmentBOOL(isDisabled);

        if (!isDisabled)
        {
            CCompartment CompartmentEmptyContext(_pThreadMgr, _tfClientId, GUID_COMPARTMENT_EMPTYCONTEXT);
            CompartmentEmptyContext._GetCompartmentBOOL(isDisabled);
        }

        if (SUCCEEDED(pDocMgrFocus->GetTop(&pContext)) && pContext != nullptr)
        {
            pContext->Release();
        }
        pDocMgrFocus->Release();
    }

    return isDisabled;
}

STDAPI BornoTSF::OnSetFocus(BOOL fForeground)
{
    fForeground;
    return S_OK;
}

STDAPI BornoTSF::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    Global::UpdateModifiers(wParam, lParam);

    if (CCandidateWindow::GetInstance().IsVisible())
    {
        if (wParam == VK_UP || wParam == VK_DOWN
            || wParam == VK_TAB || wParam == VK_SPACE || wParam == VK_RETURN)
        {
            *pIsEaten = TRUE;
            return S_OK;
        }
    }

    _KEYSTROKE_STATE KeystrokeState;
    WCHAR wch = '\0';
    UINT code = 0;
    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, &KeystrokeState);

    if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
    {
        KeystrokeState.Category = CATEGORY_COMPOSING;
        _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
    }

    return S_OK;
}

STDAPI BornoTSF::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    Global::UpdateModifiers(wParam, lParam);

    if (CCandidateWindow::GetInstance().IsVisible())
    {
        if (wParam == VK_SPACE || wParam == VK_RETURN)
        {
            // commit highlighted candidate, forward triggering key
            int selIdx = CCandidateWindow::GetInstance().GetSelectedIndex();
            UINT selectCode = (UINT)('1' + selIdx);
            Global::CandidateCommitKey = (UINT)wParam; // remember to forward after commit
            _KEYSTROKE_STATE KeystrokeState;
            KeystrokeState.Category = CATEGORY_COMPOSING;
            KeystrokeState.Function = FUNCTION_CANDIDATE_SELECT;
            _InvokeKeyHandler(pContext, selectCode, '\0', (DWORD)lParam, KeystrokeState);
            *pIsEaten = TRUE;
            return S_OK;
        }
        else if (wParam == VK_UP)
        {
            CCandidateWindow::GetInstance().SelectPrev();
            *pIsEaten = TRUE;
            return S_OK;
        }
        else if (wParam == VK_DOWN || wParam == VK_TAB)
        {
            CCandidateWindow::GetInstance().SelectNext();
            *pIsEaten = TRUE;
            return S_OK;
        }
    }

    _KEYSTROKE_STATE KeystrokeState;
    WCHAR wch = '\0';
    UINT code = 0;

    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, &KeystrokeState);

    if (*pIsEaten)
    {
        bool needInvokeKeyHandler = true;
        if (code == VK_ESCAPE)
        {
            KeystrokeState.Category = CATEGORY_COMPOSING;
        }

        if ((wch == THIRDPARTY_NEXTPAGE) || (wch == THIRDPARTY_PREVPAGE))
        {
            needInvokeKeyHandler = !((KeystrokeState.Category == CATEGORY_NONE) && (KeystrokeState.Function == FUNCTION_NONE));
        }

        if (needInvokeKeyHandler)
        {
            _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
        }
    }
    else if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
    {
        KeystrokeState.Category = CATEGORY_COMPOSING;
        _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
    }

    return S_OK;
}

STDAPI BornoTSF::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    if (pIsEaten == nullptr)
    {
        return E_INVALIDARG;
    }
    Global::UpdateModifiers(wParam, lParam);
    *pIsEaten = FALSE;
    return S_OK;
}

STDAPI BornoTSF::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    Global::UpdateModifiers(wParam, lParam);
    *pIsEaten = FALSE;
    return S_OK;
}

STDAPI BornoTSF::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten)
{
    pContext;

    if (IsEqualGUID(rguid, Global::BornoTSFGuidImeModePreserveKey))
    {
        CCompartment CompartmentKeyboardOpen(_pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        BOOL isOpen = FALSE;
        CompartmentKeyboardOpen._GetCompartmentBOOL(isOpen);
        CompartmentKeyboardOpen._SetCompartmentBOOL(!isOpen);
        Global::isImeEnabled = !isOpen;
        *pIsEaten = TRUE;
        return S_OK;
    }

    *pIsEaten = FALSE;
    return S_OK;
}

BOOL BornoTSF::_InitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
    HRESULT hr = S_OK;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
    {
        return FALSE;
    }

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

void BornoTSF::_UninitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
    {
        return;
    }

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}
