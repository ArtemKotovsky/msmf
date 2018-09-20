#include "stdafx.h"
#include "MediaSource.h"
#include "MFAttributes.h"
#include "ComUtils.h"

namespace mf
{
    MediaSource::MediaSource(const GUID& sourceType) noexcept
        : m_sourceType(sourceType)
    {
    }

    HRESULT MediaSource::OpenDeviceBySymLink(const std::wstring& symbolicLink, MFActivate& dev) const
    {
        MFActivateList devList;
        HRCHK(EnumDevice(devList));

        for (auto& device : devList)
        {
            mf::MFAttributes attr(device);
            std::wstring symLink = attr.GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK);

            if (0 == _wcsicmp(symLink.c_str(), symbolicLink.c_str()))
            {
                dev = device;
                return S_OK;
            }

            symLink = attr.GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK);

            if (0 == _wcsicmp(symLink.c_str(), symbolicLink.c_str()))
            {
                dev = device;
                return S_OK;
            }
        }

        return E_FAIL;
    }

    HRESULT MediaSource::OpenDeviceByFriendlyName(const std::wstring& friendlyName, MFActivateList& devs) const
    {
        MFActivateList devList;
        HRCHK(EnumDevice(devList));

        devs.clear();

        for (auto& dev : devList)
        {
            utils::TaskMemFreeTraits<LPWSTR>::Handle fn;

            UINT32 cchName = 0;
            HRESULT hr = dev->GetAllocatedString(
                MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                fn.ReleaseAndGetAddressOf(),
                &cchName);

            if (FAILED(hr))
            {
                continue;
            }

            if (0 == _wcsicmp(fn.Get(), friendlyName.c_str()))
            {
                devs.push_back(dev);
            }
        }

        if (devs.empty())
        {
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT MediaSource::OpenDevice(const std::wstring& device, MFActivateList& devs) const
    {
        bool foundDevice = false;
        MFActivate dev;

        //
        // Try to open device by symbolic link
        //
        HRESULT hr = OpenDeviceBySymLink(device, dev);

        if (SUCCEEDED(hr))
        {
            devs.push_back(dev);
            foundDevice = true;
        }

        //
        // Try to open device by number in device's list
        //
        ULONG deviceNumber = 0;

        try
        {
            size_t pos = 0;
            deviceNumber = std::stoul(device, &pos);
            hr = (pos != device.size()) ? E_FAIL : S_OK;
        }
        catch (const std::invalid_argument&)
        {
            // Cannot convert 'device' to a device number
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            hr = OpenDeviceByNumber(deviceNumber, dev);

            if (SUCCEEDED(hr))
            {
                devs.push_back(dev);
                foundDevice = true;
            }
        }

        //
        // Try to open device by Friendly Name
        //
        mf::MFActivateList devsByFriendlyName;
        hr = OpenDeviceByFriendlyName(device, devsByFriendlyName);

        if (SUCCEEDED(hr))
        {
            devs.insert(devs.end(), devsByFriendlyName.begin(), devsByFriendlyName.end());
            foundDevice = true;
        }

        return foundDevice ? S_OK : E_FAIL;
    }

    HRESULT MediaSource::OpenDeviceByNumber(ULONG deviceNumber, MFActivate& dev) const
    {
        MFActivateList devs;
        HRCHK(EnumDevice(devs));

        if (deviceNumber >= devs.size())
        {
            return E_FAIL;
        }

        dev = devs.at(deviceNumber);
        return S_OK;
    }

    HRESULT MediaSource::EnumDevice(MFActivateList& devs) const
    {
        ComPtr<IMFAttributes> pAttributes;
        HRCHK(MFCreateAttributes(&pAttributes, 1));

        HRCHK(pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            m_sourceType));

        UINT32 count = 0;

        utils::TaskMemFreeTraits<IMFActivate**>::Handle ppDevices;
        HR_CHECK2(MFEnumDeviceSources(pAttributes.Get(), ppDevices.GetAddressOf(), &count));

        devs.clear();
        devs.resize(count);

        for (DWORD i = 0; i < count; i++)
        {
            devs[i].Attach(ppDevices.Get()[i]);
        }

        return S_OK;
    }
}
