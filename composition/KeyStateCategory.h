// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "Globals.h"
#include "Private.h"
#include "BornoTSF.h"

class CKeyStateCategory;

class CKeyStateCategoryFactory
{
public:
    static CKeyStateCategoryFactory* Instance();
    CKeyStateCategory* MakeKeyStateCategory(KEYSTROKE_CATEGORY keyCategory, _In_ BornoTSF *pTextService);
    void Release();

protected:
    CKeyStateCategoryFactory();

private:
    static CKeyStateCategoryFactory* _instance;
};

typedef struct KeyHandlerEditSessionDTO
{
    KeyHandlerEditSessionDTO(TfEditCookie tFEC, _In_ ITfContext *pTfContext, UINT virualCode, WCHAR inputChar, KEYSTROKE_FUNCTION arrowKeyFunction)
    {
        ec = tFEC;
        pContext = pTfContext;
        code = virualCode;
        wch = inputChar;
        arrowKey = arrowKeyFunction;
    }

    TfEditCookie ec;
    ITfContext* pContext;
    UINT code;
    WCHAR wch;
    KEYSTROKE_FUNCTION arrowKey;
} KeyHandlerEditSessionDTO;

class CKeyStateCategory
{
public:
    CKeyStateCategory(_In_ BornoTSF *pTextService);

protected:
    virtual ~CKeyStateCategory(void);

public:
    HRESULT KeyStateHandler(KEYSTROKE_FUNCTION function, KeyHandlerEditSessionDTO dto);
    void Release(void);

protected:
    virtual HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyCancel(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyBackspace(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto);
    virtual HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto);

protected:
    BornoTSF* _pTextService;
};

class CKeyStateComposing : public CKeyStateCategory
{
public:
    CKeyStateComposing(_In_ BornoTSF *pTextService);

protected:
    HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyCancel(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyBackspace(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto);
    HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto);
};

class CKeyStateNull : public CKeyStateCategory
{
public:
    CKeyStateNull(_In_ BornoTSF *pTextService) : CKeyStateCategory(pTextService) {};

protected:
    HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyInput(dto); };
    HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyFinalizeTextStoreAndInput(dto); };
    HRESULT HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyFinalizeTextStore(dto); };
    HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyConvert(dto); };
    HRESULT HandleKeyCancel(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyCancel(dto); };
    HRESULT HandleKeyBackspace(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyBackspace(dto); };
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyArrow(dto); };
    HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyDoubleSingleByte(dto); };
    HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto) { return CKeyStateCategory::HandleKeyPunctuation(dto); };
};