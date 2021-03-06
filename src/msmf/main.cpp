// msmf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "msmf.h"
#include "ComUtils.h"
#include "MediaSource.h"
#include "MFAttributes.h"
#include "CaptureWindow.h"

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Shlwapi.lib")

namespace 
{
    void PrintHelp()
    {
        std::wcout << "Usage:";
        std::wcout << "\n --list \n\t Print avaliable devices list with modes\n";
        std::wcout << "\n --devices \n\t  Print avaliable devices short info\n";
        std::wcout << "\n --modes <name|ID|friendly name> \n\t Print avaliable device's modes\n";
        std::wcout << "\n --capture <device name|ID|friendly name> <StreamID> <ModeID>\n";
        std::wcout << "\n --capture <timeout in seconds> \n\t Start capture of each device\n";
    }

    std::string Utf16ToUtf8(const std::wstring& s)
    {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(s);
    }

    unsigned long StoulSafe(const std::wstring& str)
    {
        try
        {
            size_t pos = 0;
            unsigned long res = std::stoul(str, &pos);

            if (pos == str.size())
            {
                return res;
            }
        }
        catch (const std::invalid_argument&)
        {
            // stoul cannot convert
        }

        std::stringstream st;
        st << "Cannot convert '" << Utf16ToUtf8(str) << "' to digit";
        throw std::invalid_argument(st.str());
    }

    HRESULT OpenDevice(const std::wstring& device, mf::MFActivate& dev)
    {
        mf::MediaVideoSource src;
        mf::MFActivateList devs;

        HRESULT hr = src.OpenDevice(device, devs);
        
        if (FAILED(hr))
        {
            std::wcout << "Cannot open device '" << device << "'\n";
            return hr;
        }

        if (devs.size() > 1)
        {
            std::wcout << "WARNING!"
                << " Found " << devs.size() << " devices by '"
                << device << "'. Use the first one...\n";
        }

        dev = devs.at(0);
        return hr;
    }

    HRESULT DeviceModes(const std::wstring& device)
    {
        mf::MFActivate dev;
        HRCHK(OpenDevice(device, dev));
        HRCHK(console::DeviceMediaTypeList(dev, true));
        return S_OK;
    }

    HRESULT DeviceCapture(const std::wstring& device, ULONG streamID, ULONG modeID)
    {
        mf::MFActivate dev;
        HRCHK(OpenDevice(device, dev));

        mf::CaptureWindow window;
        HRCHK(console::StartCapture(window, dev, streamID, modeID, false));

        return S_OK;
    }

    void InteractiveMode()
    {
        try
        {
            HR_CHECK2(console::DeviceCaptureOneByOne(10));
            std::cout << "\nDone";
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Error: " << ex.what();
        }
        (void)std::getchar();
    }
}

int wmain(int argc, wchar_t ** argv)
{
    try
    {
        utils::CoInit coInit(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        utils::MFStartupInit mfInit(MF_VERSION);

        if (1 == argc)
        {
            InteractiveMode();
            return 0;
        }

        HRESULT hr = S_OK;
        std::wstring cmd = argv[1];

        if (0 == _wcsicmp(cmd.c_str(), L"--devices"))
        {
            hr = console::DeviceList(false);
        }
        else if (0 == _wcsicmp(cmd.c_str(), L"--list"))
        {
            hr = console::DeviceList(true);
        }
        else if (0 == _wcsicmp(cmd.c_str(), L"--modes") && argc == 3)
        {
            std::wstring device = argv[2];
            hr = DeviceModes(device);
        }
        else if (0 == _wcsicmp(cmd.c_str(), L"--capture") && argc == 5)
        {
            std::wstring device = argv[2];
            std::wstring streamID = argv[3];
            std::wstring modeID = argv[4];
            hr = DeviceCapture(device, StoulSafe(streamID), StoulSafe(modeID));
        }
        else if (0 == _wcsicmp(cmd.c_str(), L"--capture") && argc == 3)
        {
            std::wstring timeout = argv[2];
            hr = console::DeviceCaptureOneByOne(StoulSafe(timeout));
        }
        else
        {
            PrintHelp();
            return 1;
        }

        if (FAILED(hr))
        {
            std::wcout << "Error 0x" << std::hex << hr << "\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return -1;
    }

    return 0;
}

