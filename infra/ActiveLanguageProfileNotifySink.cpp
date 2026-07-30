// SPDX-License-Identifier: GPL-3.0-or-later

#include "Private.h"
#include "Globals.h"
#include "BornoTSF.h"
#include "Settings.h"
#include "CompositionProcessorEngine.h"

BOOL BornoTSF::VerifyBornoTSFCLSID(_In_ REFCLSID clsid)
{
    if (IsEqualCLSID(clsid, Global::BornoTSFCLSID))
    {
        return TRUE;
    }
    return FALSE;
}

STDAPI BornoTSF::OnActivated(_In_ REFCLSID clsid, _In_ REFGUID guidProfile, _In_ BOOL isActivated)
{
    if (FALSE == VerifyBornoTSFCLSID(clsid))
    {
        return S_OK;
    }

    if (isActivated)
    {
        LANGID langid = TEXTSERVICE_LANGID;

        if (IsEqualGUID(guidProfile, Global::BornoTSFGuidProfile))
        {
            Settings::SetCurrentLayout(BORNO_PHONETIC);
        }
        else if (IsEqualGUID(guidProfile, Global::GuidProfileAvroPhonetic))
        {
            Settings::SetCurrentLayout(BORNO_AVRO);
        }
        else if (IsEqualGUID(guidProfile, Global::GuidProfileKhipro))
        {
            Settings::SetCurrentLayout(BORNO_KHIPRO);
        }
        else if (IsEqualGUID(guidProfile, Global::GuidProfileJatiyo))
        {
            Settings::SetCurrentLayout(LAYOUT_JATIYO);
        }
        else if (IsEqualGUID(guidProfile, Global::GuidProfileProbhat))
        {
            Settings::SetCurrentLayout(LAYOUT_PROBHAT);
        }
        _AddTextProcessorEngine(langid, guidProfile);
    }

    if (nullptr == _pCompositionProcessorEngine)
    {
        return S_OK;
    }

    if (isActivated)
    {
        _pCompositionProcessorEngine->ShowAllLanguageBarIcons();
        _pCompositionProcessorEngine->ConversionModeCompartmentUpdated(_pThreadMgr);
    }
    else
    {
        _DeleteCandidateList(FALSE, nullptr);
        _pCompositionProcessorEngine->HideAllLanguageBarIcons();
    }

    return S_OK;
}


BOOL BornoTSF::_InitActiveLanguageProfileNotifySink()
{
    ITfSource* pSource = nullptr;
    BOOL ret = FALSE;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
    {
        return ret;
    }

    if (pSource->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, (ITfActiveLanguageProfileNotifySink *)this, &_activeLanguageProfileNotifySinkCookie) != S_OK)
    {
        _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    ret = TRUE;

Exit:
    pSource->Release();
    return ret;
}

void BornoTSF::_UninitActiveLanguageProfileNotifySink()
{
    ITfSource* pSource = nullptr;

    if (_activeLanguageProfileNotifySinkCookie == TF_INVALID_COOKIE)
    {
        return;
    }

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(_activeLanguageProfileNotifySinkCookie);
        pSource->Release();
    }

    _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
}
