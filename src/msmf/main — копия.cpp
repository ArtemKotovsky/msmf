// msmf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "msmf.h"


#include "ComUtils.h"
#include "MediaSource.h"
#include "MFAttributes.h"


#include "Reader.h"
#include "DirectxYUV422SemiPlanaSurface.h"
#include "DirectXWindow.h"

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Shlwapi.lib")



ComPtr<r::SourceReaderCB> gReader;
ComPtr<IMFSample> videoSample;
LONGLONG sampleTime;


bool grabFrame(IMFSourceReaderEx * videoFileSource, DWORD dwStreamIndex)
{
    bool isOpen = true;
    if (gReader)  // async "live" capture mode
    {
        HRESULT hr = 0;
        if (!gReader->m_reader)
        {
            // Initiate capturing with async callback
            gReader->m_reader = videoFileSource;
            gReader->m_dwStreamIndex = dwStreamIndex;
            if (FAILED(hr = videoFileSource->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL)))
            {
                //CV_LOG_ERROR(NULL, "videoio(MSMF): can't grab frame - initial async ReadSample() call failed: " << hr);
                gReader->m_reader = NULL;
                return false;
            }
        }
        BOOL bEOS = false;
        if (FAILED(hr = gReader->Wait(10000, videoSample, bEOS)))  // 10 sec
        {
            //CV_LOG_WARNING(NULL, "videoio(MSMF): can't grab frame. Error: " << hr);
            return false;
        }
        if (bEOS)
        {
            //CV_LOG_WARNING(NULL, "videoio(MSMF): EOS signal. Capture stream is lost");
            return false;
        }
        return true;
    }
    else if (isOpen)
    {
        DWORD streamIndex, flags;
        videoSample.Detach();
        HRESULT hr;
        for (;;)
        {
            //CV_TRACE_REGION("ReadSample");
            if (!SUCCEEDED(hr = videoFileSource->ReadSample(
                dwStreamIndex, // Stream index.
                0,             // Flags.
                &streamIndex,  // Receives the actual stream index.
                &flags,        // Receives status flags.
                &sampleTime,   // Receives the time stamp.
                &videoSample   // Receives the sample or NULL.
            )))
                break;
            if (streamIndex != dwStreamIndex)
                break;
            if (flags & (MF_SOURCE_READERF_ERROR | MF_SOURCE_READERF_ALLEFFECTSREMOVED | MF_SOURCE_READERF_ENDOFSTREAM))
                break;
            if (videoSample)
                break;
            if (flags & MF_SOURCE_READERF_STREAMTICK)
            {
                //CV_LOG_DEBUG(NULL, "videoio(MSMF): Stream tick detected. Retrying to grab the frame");
            }
        }

        if (SUCCEEDED(hr))
        {
            if (streamIndex != dwStreamIndex)
            {
                //CV_LOG_DEBUG(NULL, "videoio(MSMF): Wrong stream readed. Abort capturing");
                //close();
            }
            else if (flags & MF_SOURCE_READERF_ERROR)
            {
                //CV_LOG_DEBUG(NULL, "videoio(MSMF): Stream reading error. Abort capturing");
                //close();
            }
            else if (flags & MF_SOURCE_READERF_ALLEFFECTSREMOVED)
            {
                //CV_LOG_DEBUG(NULL, "videoio(MSMF): Stream decoding error. Abort capturing");
                //close();
            }
            else if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            {
                //sampleTime += frameStep;
                //CV_LOG_DEBUG(NULL, "videoio(MSMF): End of stream detected");
            }
            else
            {
                //sampleTime += frameStep;
                if (flags & MF_SOURCE_READERF_NEWSTREAM)
                {
                    //CV_LOG_DEBUG(NULL, "videoio(MSMF): New stream detected");
                }
                if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
                {
                   // CV_LOG_DEBUG(NULL, "videoio(MSMF): Stream native media type changed");
                }
                if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
                {
                    //CV_LOG_DEBUG(NULL, "videoio(MSMF): Stream current media type changed");
                }
                return true;
            }
        }
    }
    return false;
}

int main()
{
    //
    // 0. Devices + modes
    // 1. Device list  (Path, FriendlyName)
    // 2. Device modes (streams + modes) by [Path or ID or FriendlyName]
    // 3. Start transmitting + change modes in interactive [Path or ID or FriendlyName] [Stream ID] [Mode ID]
    //

    try
    {
        utils::CoInit coInit(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        utils::MFStartupInit mfInit(MF_VERSION);

        console::DeviceList(true);

        ComPtr<IMFAttributes> pAttributes;
        HR_CHECK2(MFCreateAttributes(&pAttributes, 1));

        // Ask for source type = video capture devices
        HR_CHECK2(pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));


        
        UINT32 count = 0;
        IMFActivate **ppDevices = NULL;
        HR_CHECK2(MFEnumDeviceSources(pAttributes.Get(), &ppDevices, &count));


        for (UINT32 idx = 0; idx < count; idx++)
        {
            IMFActivate* pDevice = ppDevices[idx];

            ComPtr<IMFSourceReader> videoFileSource2;
            ComPtr<IMFMediaSource> mSrc;
            ComPtr<IMFAttributes> srAttr;

            if (FAILED(pDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&mSrc)))
            {
                continue;
            }

            HR_CHECK2(MFCreateAttributes(&srAttr, 10));
            HR_CHECK2(srAttr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
            HR_CHECK2(srAttr->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, FALSE));
            HR_CHECK2(srAttr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, FALSE));
            HR_CHECK2(srAttr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE));


            gReader.Attach(new r::SourceReaderCB());
            HR_CHECK2(srAttr->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, (IMFSourceReaderCallback*)gReader.Get()));

            HR_CHECK2(MFCreateSourceReaderFromMediaSource(mSrc.Get(), srAttr.Get(), &videoFileSource2));

            ComPtr<IMFSourceReaderEx> videoFileSource = nullptr;
            HR_CHECK2(videoFileSource2.As(&videoFileSource));
            HR_CHECK2(videoFileSource->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, TRUE));

            DWORD dwMediaTypeTest = 0;
            DWORD dwStreamTest = 0;
            HRESULT hr = S_OK;

            std::vector<MediaType> res;

            while (SUCCEEDED(hr))
            {
                ComPtr<IMFMediaType> pType;
                hr = videoFileSource->GetNativeMediaType(dwStreamTest, dwMediaTypeTest, &pType);

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
                    res.push_back(LoadMediaTypes(pType.Get(), dwStreamTest));
                }

                ++dwMediaTypeTest;
            }

            ULONG i = 0;
            for (const auto& mt : res)
            {
                std::cout << "[" << i++ << "]"
                    << " " << mt.Height << "x" << mt.Width
                    << " " << mt.GetFps() << "@fps"
                    << " " << mt.GetFormat() << "\n";
            }

            while (1)
            {
                std::cout << "\n" << "Mode ID: ";

                ULONG32 mode = 0;
                std::cin >> mode;

                if (mode >= res.size())
                {
                    continue;
                }

                std::cout << "Select"
                    << " " << res[mode].Height << "x" << res[mode].Width
                    << " " << res[mode].GetFps() << "@fps"
                    << " " << res[mode].GetFormat() << "\n";

                SelectFormat(videoFileSource.Get() , res[mode]);

                gui::DirectxYUV422SemiPlanarSurface s;
                gui::DirectxWindow wnd(s);
                wnd.Show(0, 0, res[mode].Width, res[mode].Height, 0);

                for (;;)
                {
                    if (!grabFrame(videoFileSource.Get(), 0 /*stream ID*/))
                    {
                        continue;
                    }

                    std::lock_guard<std::mutex> lock(gReader->m_mutex);

                    if (videoSample.Get())
                    {
                        ComPtr<IMFMediaBuffer> buf;
                        if (SUCCEEDED(videoSample->ConvertToContiguousBuffer(&buf)))
                        {
                            BYTE * pbuf;
                            DWORD len = 0;
                            DWORD maxLen = 0;

                            if (SUCCEEDED(buf->Lock(&pbuf, &maxLen, &len)))
                            {
                                s.Render(pbuf);
                                buf->Unlock();
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << "ok";
        std::getchar();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        std::getchar();
        return -1;
    }

    return 0;
}

