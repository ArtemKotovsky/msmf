#include "stdafx.h"
#include "msmf.h"
#include "MediaSource.h"
#include "MFAttributes.h"
#include "CaptureWindow.h"

namespace console
{
    HRESULT DeviceList(bool verbose)
    {
        mf::MediaVideoSource sources;
        mf::MFActivateList devs;
        HRCHK(sources.EnumDevice(devs));

        size_t deviceNumber = 0;

        for (auto& dev : devs)
        {
            mf::MFAttributes attr(dev);

            std::wcout << "Device #" << deviceNumber++;
            std::wcout << "\n  Friendly name: " << attr.GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME);
            std::wcout << "\n  Sym link: " << attr.GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK);
            std::wcout << "\n  Buffered frames: " << std::dec << attr.GetUint32(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_MAX_BUFFERS);
            std::wcout << "\n  Video category: " << attr.GetGuidName(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_CATEGORY);

            if (0 == attr.GetUint32(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_HW_SOURCE))
            {
                std::wcout << "\n  Software device";
            }
            else
            {
                std::wcout << "\n  Hardware device";
            }

            std::vector<UINT8> data;
            HRESULT hr = attr.GetBlob(MF_DEVSOURCE_ATTRIBUTE_MEDIA_TYPE, data);

            if (SUCCEEDED(hr) && !data.empty())
            {
                auto typeInfo = reinterpret_cast<MFT_REGISTER_TYPE_INFO*>(data.data());
                size_t count = data.size() / sizeof(*typeInfo);
                assert(0 == (data.size() % sizeof(*typeInfo)));

                for (size_t i = 0; i < count; ++i)
                {
                    std::wcout << "\n  Media major type: " << attr.GuidToName(typeInfo[i].guidMajorType);
                    std::wcout << "\n  Media subtype:    " << attr.GuidToName(typeInfo[i].guidSubtype);
                }
            }

            std::wcout << "\n";

            if (verbose)
            {
                DeviceMediaTypeList(dev, false);
                std::wcout << "\n\n";
            }
        }

        return S_OK;
    }

    HRESULT DeviceMediaTypeList(ComPtr<IMFActivate>& pActivate, bool verbose)
    {
        ComPtr<IMFMediaSource> pSource;
        HRCHK(pActivate->ActivateObject(
            __uuidof(IMFMediaSource),
            reinterpret_cast<void**>(pSource.GetAddressOf())));

        ComPtr<IMFSourceReader> pVideoFileSource;
        HRCHK(MFCreateSourceReaderFromMediaSource(
            pSource.Get(), 
            NULL, 
            pVideoFileSource.GetAddressOf()));

        HRCHK(pVideoFileSource->SetStreamSelection(
            static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), 
            TRUE));

        DWORD dwMediaTypeTest = 0;
        DWORD dwStreamTest = 0;
        HRESULT hr = S_OK;

        while (SUCCEEDED(hr))
        {
            ComPtr<IMFMediaType> pType;
            hr = pVideoFileSource->GetNativeMediaType(
                dwStreamTest, 
                dwMediaTypeTest, 
                pType.GetAddressOf());

            if (hr == MF_E_NO_MORE_TYPES)
            {
                hr = S_OK;
                dwMediaTypeTest = 0;
                ++dwStreamTest;
                continue;
            }

            if (hr == MF_E_INVALIDSTREAMNUMBER)
            {
                break;
            }

            if (SUCCEEDED(hr))
            {
                if (verbose)
                {
                    std::wcout << "\n\n Stream ID : " << std::dec << dwStreamTest;
                    std::wcout << "\n Media type ID : " << std::dec << dwMediaTypeTest;
                }
                else
                {
                    std::wcout << "\n  [" << std::dec << dwStreamTest << ", " << dwMediaTypeTest << "]:";
                }

                if (SUCCEEDED(pType->LockStore()))
                {
                    if (verbose)
                    {
                        PrintMediaType(pType.Get());
                    }
                    else
                    {
                        PrintBaseVideoMediaType(pType.Get(), std::wcout);
                    }
                    pType->UnlockStore();
                }
            }

            ++dwMediaTypeTest;
        }

        return S_OK;
    }

    HRESULT PrintBaseVideoMediaType(IMFMediaType * pMediaType, std::wostream& st)
    {
        UINT32 count = 0;
        HRCHK(pMediaType->GetCount(&count));

        GUID guid = { 0 };

        for (UINT32 i = 0; i < count; ++i)
        {
            PROPVARIANT var;
            PropVariantInit(&var);

            if (FAILED(pMediaType->GetItemByIndex(i, &guid, &var)))
            {
                continue;
            }

            if (guid == MF_MT_FRAME_RATE && var.vt == VT_UI8)
            {
                ULONG numerator = HI32(var.uhVal.QuadPart);
                ULONG denominator = LO32(var.uhVal.QuadPart);
                double val = 0;

                if (0 != denominator)
                {
                    val = static_cast<double>(numerator) / static_cast<double>(denominator);
                }

                st << " " << val << "@FPS";
            }
            // else if (guid == MF_MT_MAJOR_TYPE && var.vt == VT_CLSID)
            // {
            //     st << " " << mf::MFAttributes::GuidToName(*var.puuid);
            // }
            else if (guid == MF_MT_SUBTYPE && var.vt == VT_CLSID)
            {
                st << " " << mf::MFAttributes::GuidToName(*var.puuid);
            }
            else if (guid == MF_MT_FRAME_SIZE && var.vt == VT_UI8)
            {
                ULONG width = HI32(var.uhVal.QuadPart);
                ULONG height = LO32(var.uhVal.QuadPart);
                st << " " << std::dec << width << " x " << std::dec << height;
            }

            PropVariantClear(&var);
        }

        return S_OK;
    }

    HRESULT PrintMediaType(IMFMediaType * pMediaType)
    {
        UINT32 count = 0;
        HRCHK(pMediaType->GetCount(&count));

        GUID guid = { 0 };

        for (UINT32 i = 0; i < count; ++i)
        {
            PROPVARIANT var;
            PropVariantInit(&var);

            if (FAILED(pMediaType->GetItemByIndex(i, &guid, &var)))
            {
                std::wcout << "\n\r " << "CANNOT GET GUID #" << std::dec << i;
                continue;
            }

            if (guid == MF_MT_FRAME_RATE && var.vt == VT_UI8)
            {
                ULONG numerator = HI32(var.uhVal.QuadPart);
                ULONG denominator = LO32(var.uhVal.QuadPart);
                double val = 0;
                
                if (0 != denominator)
                {
                    val = static_cast<double>(numerator) / static_cast<double>(denominator);
                }

                std::wcout << "\n\r FRAME_RATE : " << val;
            }
            else if (guid == MF_MT_MAJOR_TYPE && var.vt == VT_CLSID)
            {
                std::wcout << "\n\r MAJOR_TYPE : " << mf::MFAttributes::GuidToName(*var.puuid);
            }
            else if (guid == MF_MT_SUBTYPE && var.vt == VT_CLSID)
            {
                std::wcout << "\n\r SUBTYPE : " << mf::MFAttributes::GuidToName(*var.puuid);
            }
            else if (guid == MF_MT_FRAME_SIZE && var.vt == VT_UI8)
            {
                ULONG width = HI32(var.uhVal.QuadPart);
                ULONG height = LO32(var.uhVal.QuadPart);
                std::wcout << "\n\r FRAME_SIZE : " << std::dec << width << " x " << std::dec << height;
            }
            else if (guid == MF_MT_AM_FORMAT_TYPE && var.vt == VT_CLSID)
            {
                std::wcout << "\n\r AM_FORMAT_TYPE : " << mf::MFAttributes::GuidToName(*var.puuid);
            }
            else if (guid == MF_MT_DEFAULT_STRIDE && var.vt == VT_UI4)
            {
                std::wcout << "\n\r DEFAULT_STRIDE : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_FRAME_RATE_RANGE_MAX && var.vt == VT_UI8)
            {
                ULONG numerator = HI32(var.uhVal.QuadPart);
                ULONG denominator = LO32(var.uhVal.QuadPart);

                double val = 0;

                if (0 != denominator)
                {
                    val = static_cast<double>(numerator) / static_cast<double>(denominator);
                }

                std::wcout << "\n\r FRAME_RATE_RANGE_MAX : " << std::dec << val;
            }
            else if (guid == MF_MT_FRAME_RATE_RANGE_MIN && var.vt == VT_UI8)
            {
                ULONG numerator = HI32(var.uhVal.QuadPart);
                ULONG denominator = LO32(var.uhVal.QuadPart);

                double val = 0;

                if (0 != denominator)
                {
                    val = static_cast<double>(numerator) / static_cast<double>(denominator);
                }

                std::wcout << "\n\r FRAME_RATE_RANGE_MIN : " << val;
            }
            else if (guid == MF_MT_PIXEL_ASPECT_RATIO && var.vt == VT_UI8)
            {
                ULONG numerator = HI32(var.uhVal.QuadPart);
                ULONG denominator = LO32(var.uhVal.QuadPart);

                double val = 0;

                if (0 != denominator)
                {
                    val = static_cast<double>(numerator) / static_cast<double>(denominator);
                }

                std::wcout << "\n\r PIXEL_ASPECT_RATIO : " << val;
            }
            else if (guid == MF_MT_YUV_MATRIX && var.vt == VT_UI4)
            {
                std::wcout << "\n\r YUV_MATRIX : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_VIDEO_LIGHTING && var.vt == VT_UI4)
            {
                std::wcout << "\n\r VIDEO_LIGHTING : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_VIDEO_CHROMA_SITING && var.vt == VT_UI4)
            {
                std::wcout << "\n\r VIDEO_CHROMA_SITING : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_VIDEO_NOMINAL_RANGE && var.vt == VT_UI4)
            {
                std::wcout << "\n\r VIDEO_NOMINAL_RANGE : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_ALL_SAMPLES_INDEPENDENT && var.vt == VT_UI4)
            {
                std::wcout << "\n\r ALL_SAMPLES_INDEPENDENT : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_FIXED_SIZE_SAMPLES && var.vt == VT_UI4)
            {
                std::wcout << "\n\r FIXED_SIZE_SAMPLES : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_SAMPLE_SIZE && var.vt == VT_UI4)
            {
                std::wcout << "\n\r SAMPLE_SIZE : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_VIDEO_PRIMARIES && var.vt == VT_UI4)
            {
                std::wcout << "\n\r VIDEO_PRIMARIES : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_INTERLACE_MODE && var.vt == VT_UI4)
            {
                std::wcout << "\n\r INTERLACE_MODE : " << std::dec << var.ulVal;
            }
            else if (guid == MF_MT_AVG_BITRATE && var.vt == VT_UI4)
            {
                std::wcout << "\n\r AVG_BITRATE : " << std::dec << var.ulVal;
            }
            else
            {
                std::wcout << "\n\r " << "UNKNOWN GUID : " << mf::MFAttributes::GuidToName(guid);
            }

            PropVariantClear(&var);
        }

        return S_OK;
    }

    HRESULT StartCapture(mf::CaptureWindow& window, ComPtr<IMFActivate>& pActivate, ULONG streamId, ULONG mediaId, bool inThread)
    {
        ComPtr<IMFMediaSource> pSource;
        HRCHK(pActivate->ActivateObject(
            __uuidof(IMFMediaSource),
            reinterpret_cast<void**>(pSource.GetAddressOf())));

        ComPtr<IMFAttributes> pAttributes;
        HRCHK(MFCreateAttributes(&pAttributes, 10));
        HRCHK(pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
        HRCHK(pAttributes->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, FALSE));
        HRCHK(pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, FALSE));
        HRCHK(pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE));
        HRCHK(pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, &window));

        ComPtr<IMFSourceReader> pVideoFileSource;
        HRCHK(MFCreateSourceReaderFromMediaSource(
            pSource.Get(),
            pAttributes.Get(),
            pVideoFileSource.GetAddressOf()));

        HRCHK(pVideoFileSource->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), TRUE));

        ComPtr<IMFMediaType> pType;
        HRCHK(pVideoFileSource->GetNativeMediaType(
            streamId,
            mediaId,
            pType.GetAddressOf()));

        {
            mf::MFAttributes attr(pActivate);
            std::wstringstream title;

            title << attr.GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME);
            title << ":";
            PrintBaseVideoMediaType(pType.Get(), title);

            std::wcout << title.str() << "\n";
            HRCHK(window.SetTitle(title.str()));
        }

        HRCHK(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2));
        HRCHK(pVideoFileSource->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE));
        HRCHK(pVideoFileSource->SetStreamSelection(streamId, TRUE));
        HRCHK(pVideoFileSource->SetCurrentMediaType(streamId, NULL, pType.Get()));

        HRCHK(window.Show(std::move(pVideoFileSource), std::move(pType), streamId, inThread));
        return S_OK;
    }

    HRESULT DeviceCaptureOneByOne(ULONG timeoutSeconds)
    {
        //
        // Enums all device, stream and media type,
        // starts video capture for each avaliable config
        // waits for timeout and continue enumeration
        //

        mf::MediaVideoSource sources;
        mf::MFActivateList devs;
        HRCHK(sources.EnumDevice(devs));

        size_t deviceNumber = 0;

        for (auto& dev : devs)
        {
            ComPtr<IMFAttributes> pAttributes = dev;
            mf::MFAttributes attr(pAttributes);

            std::wcout << "Device #" << deviceNumber++ << " " << attr.GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME) << "\n";

            ComPtr<IMFMediaSource> pSource;
            HRCHK(dev->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void**>(pSource.GetAddressOf())));

            mf::CaptureWindow window;
            ComPtr<IMFAttributes> pVideoAttributes;
            HRCHK(MFCreateAttributes(&pVideoAttributes, 10));
            HRCHK(pVideoAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
            HRCHK(pVideoAttributes->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, FALSE));
            HRCHK(pVideoAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, FALSE));
            HRCHK(pVideoAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE));
            HRCHK(pVideoAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, &window));

            ComPtr<IMFMediaType> pType;
            ComPtr<IMFSourceReader> pVideoFileSource;
            HRCHK(MFCreateSourceReaderFromMediaSource(pSource.Get(), pVideoAttributes.Get(), pVideoFileSource.ReleaseAndGetAddressOf()));

            DWORD dwMediaTypeTest = 0;
            DWORD dwStreamTest = 0;
            HRESULT hr = S_OK;

            while (SUCCEEDED(hr))
            {
                HRCHK(pVideoFileSource->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), TRUE));
                hr = pVideoFileSource->GetNativeMediaType(dwStreamTest, dwMediaTypeTest, pType.ReleaseAndGetAddressOf());

                if (hr == MF_E_NO_MORE_TYPES)
                {
                    hr = S_OK;
                    dwMediaTypeTest = 0;
                    ++dwStreamTest;
                    continue;
                }

                if (hr == MF_E_INVALIDSTREAMNUMBER)
                {
                    break;
                }

                if (SUCCEEDED(hr))
                {
                    if (SUCCEEDED(pType->LockStore()))
                    {
                        //
                        // Print basic media type info
                        //
                        PrintBaseVideoMediaType(pType.Get(), std::wcout);
                        pType->UnlockStore();
                    }

                    //
                    // Use YUY2 as output format,
                    // because CreateOffscreenPlainSurface is failed for some formats
                    //
                    HRCHK(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2));

                    //
                    // Start the video capture
                    //
                    HRCHK(pVideoFileSource->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE));
                    HRCHK(pVideoFileSource->SetStreamSelection(dwStreamTest, TRUE));
                    HRCHK(pVideoFileSource->SetCurrentMediaType(dwStreamTest, NULL, pType.Get()));

                    hr = window.Show(pVideoFileSource, std::move(pType), dwStreamTest, true);

                    if (FAILED(hr))
                    {
                        std::wcout << "   <== error 0x" << hr;
                        hr = S_OK;
                    }
                    else if (!window.WaitForExit(timeoutSeconds * 1000))
                    {
                        window.Close();
                    }

                    std::wcout << "\n";
                }

                ++dwMediaTypeTest;
            }
        }

        return S_OK;
    }
}
