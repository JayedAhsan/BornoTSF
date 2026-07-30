// SPDX-License-Identifier: GPL-3.0-or-later

#include "KeyStateCategory.h"

CKeyStateCategoryFactory* CKeyStateCategoryFactory::_instance;

CKeyStateCategoryFactory::CKeyStateCategoryFactory()
{
    _instance = nullptr;
}

CKeyStateCategoryFactory* CKeyStateCategoryFactory::Instance()
{
    if (nullptr == _instance)
    {
        _instance = new (std::nothrow) CKeyStateCategoryFactory();
    }

    return _instance;
}

CKeyStateCategory* CKeyStateCategoryFactory::MakeKeyStateCategory(KEYSTROKE_CATEGORY keyCategory, _In_ BornoTSF *pTextService)
{
    CKeyStateCategory* pKeyState = nullptr;

    switch (keyCategory)
    {
    case CATEGORY_NONE:
        pKeyState = new (std::nothrow) CKeyStateNull(pTextService);
        break;

    case CATEGORY_COMPOSING:
        pKeyState = new (std::nothrow) CKeyStateComposing(pTextService);
        break;

    default:
        pKeyState = new (std::nothrow) CKeyStateNull(pTextService);
        break;
    }
    return pKeyState;
}

void CKeyStateCategoryFactory::Release()
{
    if (_instance)
    {
        delete _instance;
        _instance = nullptr;
    }
}

CKeyStateCategory::CKeyStateCategory(_In_ BornoTSF *pTextService)
{
    _pTextService = pTextService;
}

CKeyStateCategory::~CKeyStateCategory(void)
{
}

HRESULT CKeyStateCategory::KeyStateHandler(KEYSTROKE_FUNCTION function, KeyHandlerEditSessionDTO dto)
{
    switch(function)
    {
    case FUNCTION_INPUT:
        return HandleKeyInput(dto);

    case FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT:
        return HandleKeyFinalizeTextStoreAndInput(dto);

    case FUNCTION_FINALIZE_TEXTSTORE:
        return HandleKeyFinalizeTextStore(dto);

    case FUNCTION_CONVERT:
        return HandleKeyConvert(dto);

    case FUNCTION_CANCEL:
        return HandleKeyCancel(dto);

    case FUNCTION_BACKSPACE:
        return HandleKeyBackspace(dto);

    case FUNCTION_MOVE_LEFT:
    case FUNCTION_MOVE_RIGHT:
        return HandleKeyArrow(dto);

    case FUNCTION_DOUBLE_SINGLE_BYTE:
        return HandleKeyDoubleSingleByte(dto);

    case FUNCTION_PUNCTUATION:
        return HandleKeyPunctuation(dto);
    }
    return E_INVALIDARG;
}

void CKeyStateCategory::Release()
{
    delete this;
}

HRESULT CKeyStateCategory::HandleKeyInput(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyBackspace(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeyPunctuation(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

CKeyStateComposing::CKeyStateComposing(_In_ BornoTSF *pTextService) : CKeyStateCategory(pTextService)
{
}

HRESULT CKeyStateComposing::HandleKeyInput(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionInput(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto)
{
    _pTextService->_HandleCompositionFinalize(dto.ec, dto.pContext, FALSE);
    return _pTextService->_HandleCompositionInput(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionFinalize(dto.ec, dto.pContext, FALSE);
}

HRESULT CKeyStateComposing::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionConvert(dto.ec, dto.pContext);
}

HRESULT CKeyStateComposing::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCancel(dto.ec, dto.pContext);
}

HRESULT CKeyStateComposing::HandleKeyBackspace(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionBackspace(dto.ec, dto.pContext);
}

HRESULT CKeyStateComposing::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionArrowKey(dto.ec, dto.pContext, dto.arrowKey);
}

HRESULT CKeyStateComposing::HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionDoubleSingleByte(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyPunctuation(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionPunctuation(dto.ec, dto.pContext, dto.wch);
}
