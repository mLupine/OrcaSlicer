#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "NativeMediaCtrl.h"
#include <boost/log/trivial.hpp>

#ifdef _WIN32

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

namespace Slic3r { namespace GUI {

class NativeMediaCtrl::Impl
{
public:
    Impl(NativeMediaCtrl* owner, HWND hwnd);
    ~Impl();

    bool Load(const wxString& url);
    void Play();
    void Stop();
    void Pause();

    NativeMediaState GetState() const { return m_state.load(); }
    wxSize GetVideoSize() const { return m_video_size; }
    NativeMediaError GetLastError() const { return m_error; }

    void UpdateLayout(int width, int height);

private:
    bool InitializeMediaFoundation();
    void ShutdownMediaFoundation();
    bool CreateMediaSource(const wxString& url);
    bool CreateSourceReader();
    void RenderLoop();
    void CleanupResources();
    void NotifyStateChanged(NativeMediaState state);
    void NotifyError(NativeMediaError error);
    NativeMediaError MapHResultToError(HRESULT hr);

    NativeMediaCtrl* m_owner;
    HWND m_hwnd;
    std::atomic<NativeMediaState> m_state;
    NativeMediaError m_error;
    wxSize m_video_size;
    wxString m_url;

    ComPtr<IMFMediaSource> m_media_source;
    ComPtr<IMFSourceReader> m_source_reader;
    ComPtr<ID3D11Device> m_d3d_device;
    ComPtr<ID3D11DeviceContext> m_d3d_context;
    ComPtr<IDXGISwapChain1> m_swap_chain;
    ComPtr<ID3D11RenderTargetView> m_render_target;

    std::thread m_render_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    std::mutex m_mutex;
    bool m_mf_initialized;

    int m_width;
    int m_height;
};

NativeMediaCtrl::Impl::Impl(NativeMediaCtrl* owner, HWND hwnd)
    : m_owner(owner)
    , m_hwnd(hwnd)
    , m_state(NativeMediaState::Stopped)
    , m_error(NativeMediaError::None)
    , m_video_size(1920, 1080)
    , m_running(false)
    , m_paused(false)
    , m_mf_initialized(false)
    , m_width(640)
    , m_height(480)
{
    m_mf_initialized = InitializeMediaFoundation();
    if (m_mf_initialized) {
        BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: Media Foundation initialized";
    } else {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: Failed to initialize Media Foundation";
    }
}

NativeMediaCtrl::Impl::~Impl()
{
    Stop();
    ShutdownMediaFoundation();
}

bool NativeMediaCtrl::Impl::InitializeMediaFoundation()
{
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: MFStartup failed: " << std::hex << hr;
        return false;
    }

    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        feature_levels,
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &m_d3d_device,
        nullptr,
        &m_d3d_context
    );

    if (FAILED(hr)) {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            flags,
            feature_levels,
            ARRAYSIZE(feature_levels),
            D3D11_SDK_VERSION,
            &m_d3d_device,
            nullptr,
            &m_d3d_context
        );
    }

    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: D3D11CreateDevice failed: " << std::hex << hr;
        return false;
    }

    ComPtr<IDXGIDevice1> dxgi_device;
    hr = m_d3d_device.As(&dxgi_device);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgi_device->GetAdapter(&adapter);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIFactory2> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return false;

    RECT rect;
    GetClientRect(m_hwnd, &rect);
    m_width = std::max(1L, rect.right - rect.left);
    m_height = std::max(1L, rect.bottom - rect.top);

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width = m_width;
    swap_chain_desc.Height = m_height;
    swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    hr = factory->CreateSwapChainForHwnd(
        m_d3d_device.Get(),
        m_hwnd,
        &swap_chain_desc,
        nullptr,
        nullptr,
        &m_swap_chain
    );

    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: CreateSwapChain failed: " << std::hex << hr;
        return false;
    }

    ComPtr<ID3D11Texture2D> back_buffer;
    hr = m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr)) return false;

    hr = m_d3d_device->CreateRenderTargetView(back_buffer.Get(), nullptr, &m_render_target);
    if (FAILED(hr)) return false;

    return true;
}

void NativeMediaCtrl::Impl::ShutdownMediaFoundation()
{
    CleanupResources();

    m_render_target.Reset();
    m_swap_chain.Reset();
    m_d3d_context.Reset();
    m_d3d_device.Reset();

    if (m_mf_initialized) {
        MFShutdown();
        m_mf_initialized = false;
    }
}

void NativeMediaCtrl::Impl::CleanupResources()
{
    m_running = false;

    if (m_render_thread.joinable()) {
        m_render_thread.join();
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_source_reader.Reset();
    m_media_source.Reset();
}

bool NativeMediaCtrl::Impl::Load(const wxString& url)
{
    CleanupResources();

    m_url = url;
    m_error = NativeMediaError::None;
    NotifyStateChanged(NativeMediaState::Loading);

    if (!m_mf_initialized) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: Media Foundation not initialized";
        m_error = NativeMediaError::InternalError;
        NotifyStateChanged(NativeMediaState::Error);
        return false;
    }

    if (!CreateMediaSource(url)) {
        NotifyError(m_error);
        return false;
    }

    if (!CreateSourceReader()) {
        NotifyError(m_error);
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: Loaded URL: " << url.ToStdString();
    return true;
}

bool NativeMediaCtrl::Impl::CreateMediaSource(const wxString& url)
{
    std::wstring wide_url = url.ToStdWstring();

    ComPtr<IMFSourceResolver> resolver;
    HRESULT hr = MFCreateSourceResolver(&resolver);
    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: MFCreateSourceResolver failed: " << std::hex << hr;
        m_error = NativeMediaError::InternalError;
        return false;
    }

    MF_OBJECT_TYPE object_type = MF_OBJECT_INVALID;
    ComPtr<IUnknown> source;

    hr = resolver->CreateObjectFromURL(
        wide_url.c_str(),
        MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_READ,
        nullptr,
        &object_type,
        &source
    );

    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: CreateObjectFromURL failed: " << std::hex << hr;
        m_error = MapHResultToError(hr);
        return false;
    }

    hr = source.As(&m_media_source);
    if (FAILED(hr)) {
        m_error = NativeMediaError::InternalError;
        return false;
    }

    return true;
}

bool NativeMediaCtrl::Impl::CreateSourceReader()
{
    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateAttributes(&attributes, 3);
    if (FAILED(hr)) return false;

    hr = attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    if (FAILED(hr)) return false;

    hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    if (FAILED(hr)) return false;

    if (m_d3d_device) {
        ComPtr<IMFDXGIDeviceManager> dxgi_manager;
        UINT reset_token = 0;
        hr = MFCreateDXGIDeviceManager(&reset_token, &dxgi_manager);
        if (SUCCEEDED(hr)) {
            hr = dxgi_manager->ResetDevice(m_d3d_device.Get(), reset_token);
            if (SUCCEEDED(hr)) {
                attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxgi_manager.Get());
            }
        }
    }

    hr = MFCreateSourceReaderFromMediaSource(m_media_source.Get(), attributes.Get(), &m_source_reader);
    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: MFCreateSourceReaderFromMediaSource failed: " << std::hex << hr;
        m_error = NativeMediaError::InternalError;
        return false;
    }

    ComPtr<IMFMediaType> video_type;
    hr = MFCreateMediaType(&video_type);
    if (FAILED(hr)) return false;

    video_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    video_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

    hr = m_source_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, video_type.Get());
    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(warning) << "NativeMediaCtrl_Win: Failed to set RGB32 format, trying NV12";
        video_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
        hr = m_source_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, video_type.Get());
    }

    if (FAILED(hr)) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: SetCurrentMediaType failed: " << std::hex << hr;
        m_error = NativeMediaError::UnsupportedFormat;
        return false;
    }

    ComPtr<IMFMediaType> actual_type;
    hr = m_source_reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actual_type);
    if (SUCCEEDED(hr)) {
        UINT32 width = 0, height = 0;
        MFGetAttributeSize(actual_type.Get(), MF_MT_FRAME_SIZE, &width, &height);
        if (width > 0 && height > 0) {
            m_video_size = wxSize(width, height);
            BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: Video size: " << width << "x" << height;
        }
    }

    return true;
}

void NativeMediaCtrl::Impl::Play()
{
    if (!m_source_reader) {
        BOOST_LOG_TRIVIAL(warning) << "NativeMediaCtrl_Win: Cannot play - no source reader";
        return;
    }

    m_paused = false;

    if (m_running) {
        NotifyStateChanged(NativeMediaState::Playing);
        return;
    }

    m_running = true;
    m_render_thread = std::thread(&Impl::RenderLoop, this);

    NotifyStateChanged(NativeMediaState::Playing);
    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: Playback started";
}

void NativeMediaCtrl::Impl::Stop()
{
    m_running = false;
    m_paused = false;

    if (m_render_thread.joinable()) {
        m_render_thread.join();
    }

    NotifyStateChanged(NativeMediaState::Stopped);
    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: Playback stopped";
}

void NativeMediaCtrl::Impl::Pause()
{
    m_paused = true;
    NotifyStateChanged(NativeMediaState::Paused);
    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: Playback paused";
}

void NativeMediaCtrl::Impl::RenderLoop()
{
    while (m_running) {
        if (m_paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            continue;
        }

        ComPtr<IMFSample> sample;
        DWORD flags = 0;
        LONGLONG timestamp = 0;

        HRESULT hr;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_source_reader) break;

            hr = m_source_reader->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0,
                nullptr,
                &flags,
                &timestamp,
                &sample
            );
        }

        if (FAILED(hr)) {
            BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: ReadSample failed: " << std::hex << hr;
            m_error = MapHResultToError(hr);
            NotifyError(m_error);
            m_owner->ScheduleRetry();
            break;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Win: End of stream";
            NotifyStateChanged(NativeMediaState::Stopped);
            break;
        }

        if (flags & MF_SOURCE_READERF_ERROR) {
            BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Win: Stream error";
            m_error = NativeMediaError::DecoderError;
            NotifyError(m_error);
            m_owner->ScheduleRetry();
            break;
        }

        if (sample && m_swap_chain && m_d3d_context && m_render_target) {
            ComPtr<IMFMediaBuffer> buffer;
            hr = sample->GetBufferByIndex(0, &buffer);
            if (SUCCEEDED(hr)) {
                BYTE* data = nullptr;
                DWORD length = 0;
                hr = buffer->Lock(&data, nullptr, &length);
                if (SUCCEEDED(hr) && data) {
                    float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                    m_d3d_context->ClearRenderTargetView(m_render_target.Get(), clear_color);
                    m_swap_chain->Present(1, 0);
                    buffer->Unlock();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void NativeMediaCtrl::Impl::UpdateLayout(int width, int height)
{
    if (width <= 0 || height <= 0) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    m_width = width;
    m_height = height;

    if (m_swap_chain && m_d3d_device) {
        m_render_target.Reset();

        HRESULT hr = m_swap_chain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        if (SUCCEEDED(hr)) {
            ComPtr<ID3D11Texture2D> back_buffer;
            hr = m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
            if (SUCCEEDED(hr)) {
                m_d3d_device->CreateRenderTargetView(back_buffer.Get(), nullptr, &m_render_target);
            }
        }
    }
}

NativeMediaError NativeMediaCtrl::Impl::MapHResultToError(HRESULT hr)
{
    switch (hr) {
        case MF_E_NET_NOCONNECTION:
        case E_ACCESSDENIED:
            return NativeMediaError::NetworkUnreachable;
        case MF_E_NET_TIMEOUT:
            return NativeMediaError::ConnectionTimeout;
        case MF_E_UNSUPPORTED_FORMAT:
        case MF_E_INVALIDMEDIATYPE:
            return NativeMediaError::UnsupportedFormat;
        case MF_E_SOURCERESOLVER_MUTUALLY_EXCLUSIVE_FLAGS:
        case MF_E_UNSUPPORTED_SCHEME:
            return NativeMediaError::StreamNotFound;
        default:
            return NativeMediaError::InternalError;
    }
}

void NativeMediaCtrl::Impl::NotifyStateChanged(NativeMediaState state)
{
    m_state = state;
    wxCommandEvent event(EVT_NATIVE_MEDIA_STATE_CHANGED);
    event.SetEventObject(m_owner);
    event.SetInt(static_cast<int>(state));
    wxPostEvent(m_owner, event);
}

void NativeMediaCtrl::Impl::NotifyError(NativeMediaError error)
{
    m_state = NativeMediaState::Error;
    wxCommandEvent event(EVT_NATIVE_MEDIA_ERROR);
    event.SetEventObject(m_owner);
    event.SetInt(static_cast<int>(error));
    wxPostEvent(m_owner, event);
}

NativeMediaCtrl::NativeMediaCtrl(wxWindow* parent)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
    , m_retry_enabled(true)
    , m_retry_count(0)
{
    SetBackgroundColour(*wxBLACK);
    m_retry_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &NativeMediaCtrl::OnRetryTimer, this, m_retry_timer.GetId());

    HWND hwnd = (HWND)GetHandle();
    m_impl = std::make_unique<Impl>(this, hwnd);
}

NativeMediaCtrl::~NativeMediaCtrl()
{
    m_retry_timer.Stop();
}

bool NativeMediaCtrl::Load(const wxString& url)
{
    if (!IsSupported(url)) {
        BOOST_LOG_TRIVIAL(warning) << "NativeMediaCtrl: Unsupported URL format: " << url.ToStdString();
        return false;
    }

    m_current_url = url;
    ResetRetryState();

    StreamType type = DetectStreamType(url);
    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl: Loading " << StreamTypeToString(type).ToStdString()
                            << " stream: " << url.ToStdString();

    return m_impl->Load(url);
}

void NativeMediaCtrl::Play()
{
    if (m_impl) {
        m_impl->Play();
    }
}

void NativeMediaCtrl::Stop()
{
    m_retry_timer.Stop();
    ResetRetryState();
    if (m_impl) {
        m_impl->Stop();
    }
}

void NativeMediaCtrl::Pause()
{
    if (m_impl) {
        m_impl->Pause();
    }
}

NativeMediaState NativeMediaCtrl::GetState() const
{
    return m_impl ? m_impl->GetState() : NativeMediaState::Stopped;
}

wxSize NativeMediaCtrl::GetVideoSize() const
{
    return m_impl ? m_impl->GetVideoSize() : wxSize(1920, 1080);
}

NativeMediaError NativeMediaCtrl::GetLastError() const
{
    return m_impl ? m_impl->GetLastError() : NativeMediaError::None;
}

void NativeMediaCtrl::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    wxWindow::DoSetSize(x, y, width, height, sizeFlags);
    if (m_impl && width > 0 && height > 0) {
        m_impl->UpdateLayout(width, height);
    }
}

}} // namespace Slic3r::GUI

#endif // _WIN32
