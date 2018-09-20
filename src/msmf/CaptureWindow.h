#pragma once
#include <windows.h>
#include <shlwapi.h>
#include <wrl.h>
#include <wrl/client.h>
#include <mfreadwrite.h>
#include <d3d9.h>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include "ComUtils.h"

#pragma comment(lib, "d3d9.lib")

namespace mf
{
    class CaptureWindow : public IMFSourceReaderCallback
    {
    public:
        CaptureWindow() noexcept;
        ~CaptureWindow();

        CaptureWindow(CaptureWindow&&) = delete;
        CaptureWindow& operator=(CaptureWindow&&) = delete;

        HRESULT Show(
            ComPtr<IMFSourceReader> pVideoSource,
            ComPtr<IMFMediaType> pType,
            ULONG streamIndex,
            bool showInThread);

        HRESULT Close() noexcept;
        HRESULT SetTitle(std::wstring title);

        BOOL WaitForExit(ULONG timeout);
        HWND GetHwnd();

    private:
        HRESULT AttachWindow();
        HRESULT Render(const void* buffer, LONG pitch);

        HRESULT CreateWnd();
        HRESULT DestroyWnd();
        HRESULT ShowModalImpl(HANDLE readyEvent);

        static CaptureWindow* GetThis(HWND hwnd);
        static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam);
        static VOID CALLBACK FpsTimer(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);

    private:
        STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override;
        STDMETHODIMP_(ULONG) AddRef() override;
        STDMETHODIMP_(ULONG) Release() override;
        STDMETHODIMP OnReadSample(
            HRESULT hrStatus, 
            DWORD dwStreamIndex, 
            DWORD dwStreamFlags, 
            LONGLONG llTimestamp, 
            IMFSample *pSample) override;
        STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) override;
        STDMETHODIMP OnFlush(DWORD streamIndex) override;

    private:
        std::thread m_thread;
        std::shared_mutex m_mutex;
        std::wstring m_title;

        ComPtr<IDirect3D9> m_pDirect3D9;
        ComPtr<IDirect3DDevice9> m_pDirect3DDevice;
        ComPtr<IDirect3DSurface9> m_pDirect3DSurface;
        ComPtr<IMFSourceReader> m_pVideoSource;

        D3DFORMAT m_format;
        HWND m_hwnd;
        ULONG m_width;
        ULONG m_height;
        ULONG m_streamIndex;
        std::atomic<uint64_t> m_framesPrev;
        std::atomic<uint64_t> m_frames;
    };
}
