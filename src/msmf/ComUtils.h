#pragma once

#include <exception>
#include <sstream>
#include <winbase.h>
#include <mfapi.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#define HR_THROW($$hr, ...) {   \
    std::stringstream $st;      \
    $st << __VA_ARGS__;         \
    throw utils::HresultException($$hr, $st.str().c_str()); \
}

#define HR_CHECK($$hr, ...) {   \
    HRESULT $hr = ($$hr);       \
    if (FAILED($hr)) HR_THROW($hr, __VA_ARGS__ << " HR=0x" << std::hex << $hr); \
}

#define HR_CHECK2($$hr) HR_CHECK($$hr, #$$hr);

#define HRCHK($$hr, ...) {      \
    HRESULT $hr = ($$hr);       \
    if (FAILED($hr)) { /*__debugbreak();*/ return $hr; }\
}

using Microsoft::WRL::ComPtr;

namespace utils
{
    class HresultException : public std::runtime_error
    {
    public:
        HresultException(HRESULT hr)
            : HresultException(hr, "HresultException")
        {
        }

        HresultException(HRESULT hr, const char* message)
            : std::runtime_error(message)
            , m_hr(hr)
        {
        }

        HRESULT GetError() const
        {
            return m_hr;
        }

    private:
        HRESULT m_hr;
    };

    template<typename T>
    struct TaskMemFreeTraits
    {
        typedef T Type;

        inline static bool Close(_In_ Type h) throw()
        {
            CoTaskMemFree(h);
            return true;
        }

        inline static T GetInvalidValue() throw()
        {
            return nullptr;
        }

        using Handle = Microsoft::WRL::Wrappers::HandleT<TaskMemFreeTraits>;
    };

    struct CoInit
    {
        CoInit(DWORD dwCoInit)
        {
            HR_CHECK2(CoInitializeEx(NULL, dwCoInit));
        }

        ~CoInit()
        {
            CoUninitialize();
        }
    };

    struct MFStartupInit
    {
        MFStartupInit(ULONG version, DWORD dwFlags = MFSTARTUP_FULL)
        {
            HR_CHECK2(MFStartup(version, dwFlags));
        }

        ~MFStartupInit()
        {
            MFShutdown();
        }
    };
}