// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

class CTfInputProcessorProfile
{
public:
    CTfInputProcessorProfile();
    ~CTfInputProcessorProfile();

    HRESULT CreateInstance();
    HRESULT GetCurrentLanguage(_Out_ LANGID *plangid);
    HRESULT GetDefaultLanguageProfile(LANGID langid, REFGUID catid, _Out_ CLSID *pclsid, _Out_ GUID *pguidProfile);
    HRESULT GetActiveLanguageProfile(REFCLSID rclsid, _Out_ LANGID *plangid, _Out_ GUID *pguidProfile);

private:
    ITfInputProcessorProfiles* _pInputProcessorProfile;
};
