#include "stdafx.h"
#include "CaptureWindow.h"

using namespace Microsoft::WRL::Wrappers;

namespace mf
{
    CaptureWindow::CaptureWindow() noexcept
        : m_hwnd(nullptr)
        , m_width(0)
        , m_height(0)
        , m_framesPrev(0)
        , m_frames(0)
        , m_format(D3DFMT_UNKNOWN)
        , m_streamIndex(0)
    {
    }

    CaptureWindow::~CaptureWindow()
    {
        Close();
    }

    HRESULT CaptureWindow::Show(ComPtr<IMFSourceReader> pVideoSource, ComPtr<IMFMediaType> pType, ULONG streamIndex, bool showInThread)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        if (m_thread.joinable())
        {
            if (HWND hwnd = m_hwnd)
            {
                SetForegroundWindow(hwnd);
                return S_FALSE;
            }
            m_thread.join();
        }

        //
        // https://docs.microsoft.com/en-us/windows/desktop/medfound/video-subtype-guids
        // Video formats are often represented by FOURCCs or D3DFORMAT values. 
        // A range of GUIDs is reserved for representing these values as subtypes.
        //
        GUID subtype;
        HR_CHECK2(pType->GetGUID(MF_MT_SUBTYPE, &subtype));
        m_format = static_cast<D3DFORMAT>(subtype.Data1);

        UINT64 resolution = 0;
        HR_CHECK2(pType->GetUINT64(MF_MT_FRAME_SIZE, &resolution));

        m_width = HI32(resolution);
        m_height = LO32(resolution);
        m_framesPrev = 0;
        m_frames = 0;
        m_streamIndex = streamIndex;
        m_pVideoSource = pVideoSource;

        if (0 == m_width || 0 == m_height)
        {
            HR_CHECK(E_INVALIDARG, "invalid width or height");
        }

        if (!showInThread)
        {
            lock.unlock();
            HRESULT hr = ShowModalImpl(nullptr);

            lock.lock();
            DestroyWnd();
            return hr;
        }

        Event event(CreateEvent(NULL, FALSE, FALSE, NULL));
        if (!event.IsValid())
        {
            const DWORD err = GetLastError();
            return HRESULT_FROM_WIN32(err);
        }

        m_thread = std::thread(&CaptureWindow::ShowModalImpl, this, event.Get());

        HANDLE handles[] = { event.Get(), m_thread.native_handle() };
        lock.unlock();

        const DWORD res = WaitForMultipleObjects(ARRAYSIZE(handles), &handles[0], FALSE, INFINITE);
        
        lock.lock();
        if (res != WAIT_OBJECT_0)
        {
            DestroyWnd();
            m_thread.join();
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT CaptureWindow::Close() noexcept
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        if (m_hwnd)
        {
            if (!PostMessageW(m_hwnd, WM_CLOSE, 0, 0))
            {
                return E_FAIL;
            }
        }

        auto thread = std::move(m_thread);
        lock.unlock();

        if (thread.joinable())
        {
            thread.join();
        }
        
        return S_OK;
    }

    HRESULT CaptureWindow::SetTitle(std::wstring title)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_title = std::move(title);
        return S_OK;
    }

    BOOL CaptureWindow::WaitForExit(ULONG timeout)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        HANDLE hThread = m_thread.native_handle();
        lock.unlock();

        if (!hThread)
        {
            //
            // Running not in thread or closed
            //
            return TRUE;
        }

        DWORD res = WaitForSingleObject(hThread, timeout);
        return res == WAIT_OBJECT_0;
    }

    HWND CaptureWindow::GetHwnd()
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_hwnd;
    }

    HRESULT CaptureWindow::AttachWindow()
    {
        assert(m_width && m_height && m_format && m_hwnd);

        m_pDirect3DSurface.Reset();
        m_pDirect3DDevice.Reset();
        m_pDirect3D9.Reset();

        ComPtr<IDirect3D9> direct3D9 = Direct3DCreate9(D3D_SDK_VERSION);
        if (!direct3D9)
        {
            return E_FAIL;
        }

        D3DPRESENT_PARAMETERS d3dpp = { 0 };
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

        ComPtr<IDirect3DDevice9> direct3DDevice;
        HRCHK(IDirect3D9_CreateDevice(direct3D9
            , D3DADAPTER_DEFAULT
            , D3DDEVTYPE_HAL
            , m_hwnd
            , D3DCREATE_SOFTWARE_VERTEXPROCESSING
            , &d3dpp
            , &direct3DDevice));

        ComPtr<IDirect3DSurface9> direct3DSurfaceRender;
        HRESULT hr = IDirect3DDevice9_CreateOffscreenPlainSurface(direct3DDevice
            , m_width
            , m_height
            , m_format
            , D3DPOOL_DEFAULT
            , &direct3DSurfaceRender
            , NULL);
        
        if (FAILED(hr))
        {
            return hr;
        }

        m_pDirect3D9.Swap(direct3D9);
        m_pDirect3DDevice.Swap(direct3DDevice);
        m_pDirect3DSurface.Swap(direct3DSurfaceRender);
        return S_OK;
    }

    HRESULT CaptureWindow::Render(const void* buffer, LONG pitch)
    {
        m_frames += 1;

        if (!m_pDirect3DSurface || !m_pDirect3DDevice)
        {
            return E_FAIL;
        }

        D3DLOCKED_RECT d3dRect = {};
        HRCHK(IDirect3DSurface9_LockRect(m_pDirect3DSurface
            , &d3dRect
            , NULL
            , D3DLOCK_DONOTWAIT));

        auto dest = static_cast<char*>(d3dRect.pBits);
        auto src = static_cast<const char*>(buffer);

        if (!dest || !src)
        {
            // check dest and scr just for VS static analyzer 
            return E_FAIL;
        }

        for (ULONG row = 0; row < m_height; ++row)
        {
            assert(pitch <= d3dRect.Pitch);
            memcpy(dest, src, pitch);
            dest += d3dRect.Pitch;
            src += pitch;
        }

        HRCHK(IDirect3DSurface9_UnlockRect(m_pDirect3DSurface));
        HRCHK(IDirect3DDevice9_Clear(m_pDirect3DDevice
            , 0
            , NULL
            , D3DCLEAR_TARGET
            , D3DCOLOR_XRGB(0, 0, 0)
            , 1.0f
            , 0));
        HRCHK(IDirect3DDevice9_BeginScene(m_pDirect3DDevice));

        IDirect3DSurface9 * pBackBuffer = NULL;
        HRCHK(IDirect3DDevice9_GetBackBuffer(m_pDirect3DDevice
            , 0
            , 0
            , D3DBACKBUFFER_TYPE_MONO
            , &pBackBuffer));

        HRCHK(IDirect3DDevice9_StretchRect(m_pDirect3DDevice
            , m_pDirect3DSurface.Get()
            , NULL
            , pBackBuffer
            , NULL
            , D3DTEXF_LINEAR));

        HRCHK(IDirect3DDevice9_EndScene(m_pDirect3DDevice));
        HRCHK(IDirect3DDevice9_Present(m_pDirect3DDevice
            , NULL
            , NULL
            , NULL
            , NULL));

        return S_OK;
    }

    HRESULT CaptureWindow::CreateWnd()
    {
        if (m_hwnd)
        {
            return E_FAIL;
        }

        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpfnWndProc = WndProc;
        wc.lpszClassName = L"msmf capture window";
        wc.style = CS_HREDRAW | CS_VREDRAW;
        RegisterClassEx(&wc);

        const DWORD wndStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        RECT rect = { 0 };
        rect.right = m_width;
        rect.bottom = m_height;

        if (!AdjustWindowRect(&rect, wndStyle, FALSE))
        {
            const DWORD err = GetLastError();
            return HRESULT_FROM_WIN32(err);
        };

        m_hwnd = CreateWindowW(wc.lpszClassName
            , m_title.c_str()
            , wndStyle
            , 0, 0
            , rect.right - rect.left
            , rect.bottom - rect.top
            , NULL, NULL, NULL, NULL);

        if (!m_hwnd)
        {
            const DWORD err = GetLastError();
            return HRESULT_FROM_WIN32(err);
        }

        return S_OK;
    }

    HRESULT CaptureWindow::DestroyWnd()
    {
        HWND hwnd = m_hwnd;
        m_hwnd = nullptr;

        if (m_pVideoSource)
        {
            m_pVideoSource->Flush(m_streamIndex);
            m_pVideoSource->SetStreamSelection(m_streamIndex, FALSE);
        }

        m_pVideoSource.Reset();
        m_pDirect3DSurface.Reset();
        m_pDirect3DDevice.Reset();
        m_pDirect3D9.Reset();

        if (hwnd && !DestroyWindow(hwnd))
        {
            const DWORD err = GetLastError();
            return HRESULT_FROM_WIN32(err);
        }

        return S_OK;
    }

    HRESULT CaptureWindow::ShowModalImpl(HANDLE readyEvent)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        HRCHK(CreateWnd());
        assert(m_hwnd);

        if (0 != SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<ULONG_PTR>(this)))
        {
            const DWORD err = GetLastError();
            return HRESULT_FROM_WIN32(err);
        }

        if (0 == SetTimer(m_hwnd, 0, 1000, FpsTimer))
        {
            const DWORD err = GetLastError();
            return HRESULT_FROM_WIN32(err);
        }

        HRCHK(AttachWindow());
        HRCHK(m_pVideoSource->ReadSample(m_streamIndex, 0, NULL, NULL, NULL, NULL));

        HWND hwnd = m_hwnd;
        lock.unlock();

        if (readyEvent)
        {
            SetEvent(readyEvent);
            readyEvent = NULL;
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg = { 0 };
        BOOL ret = FALSE;

        while ((ret = GetMessage(&msg, hwnd, 0, 0)) != 0)
        {
            if (ret == -1)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return S_OK;
    }

    CaptureWindow* CaptureWindow::GetThis(HWND hwnd)
    {
        return reinterpret_cast<CaptureWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    LRESULT WINAPI CaptureWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
    {
        auto pThis = GetThis(hwnd);

        switch (msg)
        {
        case WM_CLOSE:
        {
            std::unique_lock<std::shared_mutex> lock(pThis->m_mutex);
            pThis->DestroyWnd();
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wparma, lparam);
    }

    VOID CALLBACK CaptureWindow::FpsTimer(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
    {
        UNREFERENCED_PARAMETER(msg);
        UNREFERENCED_PARAMETER(idEvent);
        UNREFERENCED_PARAMETER(dwTime);

        auto pThis = GetThis(hwnd);

        const uint64_t curr = pThis->m_frames;
        const uint64_t fps = curr - pThis->m_framesPrev;
        pThis->m_framesPrev = curr;

        std::wstringstream st;
        std::shared_lock<std::shared_mutex> lock(pThis->m_mutex);
        st << pThis->m_title << " real FPS " << fps;
        lock.unlock();

        SetWindowTextW(hwnd, st.str().c_str());
    }

    STDMETHODIMP CaptureWindow::QueryInterface(REFIID iid, void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CaptureWindow, IMFSourceReaderCallback),
            { 0 },
        };
        return QISearch(this, qit, iid, ppv);
    }

    STDMETHODIMP_(ULONG) CaptureWindow::AddRef()
    {
        return 1;
    }

    STDMETHODIMP_(ULONG) CaptureWindow::Release()
    {
        return 1;
    }

    STDMETHODIMP CaptureWindow::OnReadSample(
        HRESULT hrStatus, 
        DWORD dwStreamIndex, 
        DWORD dwStreamFlags, 
        LONGLONG llTimestamp, 
        IMFSample *pSample)
    {
        UNREFERENCED_PARAMETER(llTimestamp);

        if (FAILED(hrStatus))
        {
            std::wstringstream st;
            st << "OnReadSample error 0x" << std::hex << hrStatus;
            MessageBoxW(GetHwnd(), st.str().c_str(), NULL, MB_OK | MB_ICONERROR);
            Close();
            return hrStatus;
        }

        if (MF_SOURCE_READERF_ENDOFSTREAM & dwStreamFlags)
        {
            MessageBoxW(GetHwnd(), L"OnReadSample end of stream", NULL, MB_OK | MB_ICONINFORMATION);
            Close();
            return S_OK;
        }

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        if (pSample)
        {
            ComPtr<IMFMediaBuffer> buffer;

            if (SUCCEEDED(pSample->ConvertToContiguousBuffer(&buffer)))
            {
                ComPtr<IMF2DBuffer> buffer2d;
                PBYTE pBuffer = nullptr;
                LONG pinch = 0;

                HRESULT hr = buffer->QueryInterface(__uuidof(IMF2DBuffer), reinterpret_cast<void**>(buffer2d.ReleaseAndGetAddressOf()));
                
                if (SUCCEEDED(hr) && SUCCEEDED(buffer2d->Lock2D(&pBuffer, &pinch)))
                {
                    Render(pBuffer, pinch);
                    buffer2d->Unlock2D();
                }
            }
        }

        if (m_hwnd)
        {
            HRCHK(m_pVideoSource->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL));
        }
        return S_OK;
    }

    STDMETHODIMP CaptureWindow::OnEvent(DWORD, IMFMediaEvent *)
    {
        return S_OK;
    }

    STDMETHODIMP CaptureWindow::OnFlush(DWORD streamIndex)
    {
        UNREFERENCED_PARAMETER(streamIndex);
        return S_OK;
    }
}
