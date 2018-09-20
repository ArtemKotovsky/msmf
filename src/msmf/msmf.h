#pragma once
#include "ComUtils.h"
#include "CaptureWindow.h"

namespace console
{
    HRESULT DeviceList(bool verbose);
    HRESULT DeviceMediaTypeList(ComPtr<IMFActivate>& pActivate, bool verbose);

    HRESULT PrintBaseVideoMediaType(IMFMediaType * pMediaType, std::wostream& st);
    HRESULT PrintMediaType(IMFMediaType * pMediaType);

    HRESULT StartCapture(mf::CaptureWindow& window, ComPtr<IMFActivate>& pActivate, ULONG streamId, ULONG mediaId, bool inThread);
    HRESULT DeviceCaptureOneByOne(ULONG timeoutSeconds);
}
