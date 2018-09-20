#pragma once

#include <vector>
#include <mfidl.h>
#include "ComUtils.h"

namespace mf
{
    using MFMediaSource = ComPtr<IMFMediaSource>;
    using MFMediaSourceList = std::vector<MFMediaSource>;

    using MFActivate = ComPtr<IMFActivate>;
    using MFActivateList = std::vector<MFActivate>;

    class MediaSource
    {
    public:
        explicit MediaSource(const GUID& sourceType) noexcept;

        HRESULT OpenDeviceBySymLink(const std::wstring& symbolicLink, MFActivate& dev) const;
        HRESULT OpenDeviceByNumber(ULONG deviceNumber, MFActivate& dev) const;
        HRESULT OpenDeviceByFriendlyName(const std::wstring& friendlyName, MFActivateList& devs) const;
        HRESULT OpenDevice(const std::wstring& device, MFActivateList& devs) const;

        HRESULT EnumDevice(MFActivateList& devs) const;

    private:
        const GUID& m_sourceType;
    };

    struct MediaVideoSource : MediaSource
    {
        MediaVideoSource() noexcept
            : MediaSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
        {
        }
    };
}
