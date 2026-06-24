#include "base_window.h"
#include "d3d_manager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <stdio.h>

namespace UI {

BaseWindow::BaseWindow() {}

BaseWindow::~BaseWindow()
{
    if (m_hwnd)
    {
        // Clear the user data mapping to prevent WndProc from accessing this instance during destruction
        ::SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, 0);
        ::DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    // Clean up ImGui resources for this window
    ImGuiContext* prevContext = ImGui::GetCurrentContext();
    if (m_imguiContext != nullptr)
    {
        ImGui::SetCurrentContext(m_imguiContext);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(m_imguiContext);
        m_imguiContext = nullptr;
    }
    if (prevContext != nullptr && prevContext != m_imguiContext)
    {
        ImGui::SetCurrentContext(prevContext);
    }
    else
    {
        ImGui::SetCurrentContext(nullptr);
    }

    // Clean up swap chain and render target view
    CleanupRenderTarget();
    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }

    // Release DirectX global reference
    D3DManager::Shutdown();
}

bool BaseWindow::Create()
{
    // Initialize global D3D manager
    if (!D3DManager::Initialize())
    {
        return false;
    }

    // Query configuration from derived window class
    DWORD style = 0;
    DWORD exStyle = 0;
    int width = 800;
    int height = 600;
    const wchar_t* className = L"BaseWindowClass";
    const wchar_t* windowTitle = L"Window";
    ConfigureWindow(style, exStyle, width, height, className, windowTitle);

    // Get monitor DPI scaling
    m_dpiScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Register Win32 class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC | CS_DROPSHADOW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = ::GetModuleHandleW(nullptr);
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = className;
    ::RegisterClassExW(&wc);

    // Scale dimensions
    int scaledWidth = static_cast<int>(width * m_dpiScale);
    int scaledHeight = static_cast<int>(height * m_dpiScale);

    // Create the HWND
    m_hwnd = ::CreateWindowExW(
        exStyle,
        className,
        windowTitle,
        style,
        100, 100,
        scaledWidth, scaledHeight,
        nullptr, nullptr,
        wc.hInstance,
        this // Pass 'this' as context pointer
    );

    if (m_hwnd == nullptr)
    {
        D3DManager::Shutdown();
        return false;
    }

    // Setup swap chain for this HWND
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGIDevice* pDXGIDevice = nullptr;
    IDXGIAdapter* pDXGIAdapter = nullptr;
    IDXGIFactory* pIDXGIFactory = nullptr;
    
    HRESULT hr = D3DManager::GetDevice()->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    if (SUCCEEDED(hr))
    {
        hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
    }
    if (SUCCEEDED(hr))
    {
        hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
    }
    if (SUCCEEDED(hr))
    {
        hr = pIDXGIFactory->CreateSwapChain(D3DManager::GetDevice(), &sd, &m_swapChain);
    }
    
    if (pIDXGIFactory) pIDXGIFactory->Release();
    if (pDXGIAdapter) pDXGIAdapter->Release();
    if (pDXGIDevice) pDXGIDevice->Release();

    if (FAILED(hr) || m_swapChain == nullptr)
    {
        ::DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        D3DManager::Shutdown();
        return false;
    }

    CreateRenderTarget();

    // Setup private ImGui Context for this window
    m_imguiContext = ImGui::CreateContext();
    ImGuiContext* prevContext = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(m_imguiContext);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Apply dark style
    ImGui::StyleColorsDark();

    // Scale sizing according to DPI and enable anti-aliasing features
    ImGuiStyle& styleVar = ImGui::GetStyle();
    styleVar.ScaleAllSizes(m_dpiScale);
    styleVar.FontScaleDpi = m_dpiScale;
    styleVar.AntiAliasedLines = true;
    styleVar.AntiAliasedLinesUseTex = true;
    styleVar.AntiAliasedFill = true;

    // Load vector font for smooth text rendering
    char windowsDir[MAX_PATH];
    if (::GetWindowsDirectoryA(windowsDir, MAX_PATH) > 0)
    {
        char fontPath[MAX_PATH];
        ::sprintf_s(fontPath, MAX_PATH, "%s\\Fonts\\segoeui.ttf", windowsDir);
        ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 17.0f * m_dpiScale);
        if (font == nullptr)
        {
            ::sprintf_s(fontPath, MAX_PATH, "%s\\Fonts\\arial.ttf", windowsDir);
            io.Fonts->AddFontFromFileTTF(fontPath, 17.0f * m_dpiScale);
        }
    }

    // Initialize ImGui backends for this context and HWND
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(D3DManager::GetDevice(), D3DManager::GetContext());

    // Restore previous ImGui context
    ImGui::SetCurrentContext(prevContext);

    // Call derived class post-initialization logic
    OnInitialize();

    return true;
}

void BaseWindow::RunStep()
{
    if (m_shouldClose)
    {
        return;
    }

    // Skip rendering if window is occluded
    if (m_swapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
    {
        ::Sleep(10);
        return;
    }
    m_swapChainOccluded = false;

    // Handle swap chain resizing if triggered in message handler
    if (m_resizeWidth != 0 && m_resizeHeight != 0)
    {
        CleanupRenderTarget();
        m_swapChain->ResizeBuffers(0, m_resizeWidth, m_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        m_resizeWidth = m_resizeHeight = 0;
        CreateRenderTarget();
    }

    // Switch to our window's ImGui context
    ImGuiContext* prevContext = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(m_imguiContext);

    // Start frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Call derived class to draw UI
    OnDraw();

    // Render ImGui data
    ImGui::Render();

    // Render to our render target
    const float clearColorWithAlpha[4] = { m_clearColor.x * m_clearColor.w, m_clearColor.y * m_clearColor.w, m_clearColor.z * m_clearColor.w, m_clearColor.w };
    ID3D11DeviceContext* ctx = D3DManager::GetContext();
    ctx->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    ctx->ClearRenderTargetView(m_renderTargetView, clearColorWithAlpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present with vsync
    HRESULT hr = m_swapChain->Present(1, 0);
    m_swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

    // Restore previous ImGui context
    ImGui::SetCurrentContext(prevContext);
}

bool BaseWindow::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer)
    {
        D3DManager::GetDevice()->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
        pBackBuffer->Release();
        return true;
    }
    return false;
}

void BaseWindow::CleanupRenderTarget()
{
    if (m_renderTargetView)
    {
        m_renderTargetView->Release();
        m_renderTargetView = nullptr;
    }
}

LRESULT WINAPI BaseWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BaseWindow* pThis = reinterpret_cast<BaseWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE)
    {
        LPCREATESTRUCTW createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        pThis = reinterpret_cast<BaseWindow*>(createStruct->lpCreateParams);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hwnd = hwnd;
    }

    if (pThis != nullptr)
    {
        ImGuiContext* prevContext = ImGui::GetCurrentContext();
        if (pThis->m_imguiContext != nullptr)
        {
            ImGui::SetCurrentContext(pThis->m_imguiContext);
        }

        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        {
            ImGui::SetCurrentContext(prevContext);
            return true;
        }

        // Give the derived class a chance to handle the message
        LRESULT res = pThis->OnMessage(hwnd, msg, wParam, lParam);
        if (res != 0)
        {
            ImGui::SetCurrentContext(prevContext);
            return res;
        }

        switch (msg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                pThis->m_resizeWidth = (UINT)LOWORD(lParam);
                pThis->m_resizeHeight = (UINT)HIWORD(lParam);
            }
            ImGui::SetCurrentContext(prevContext);
            return 0;
            
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            {
                ImGui::SetCurrentContext(prevContext);
                return 0;
            }
            break;
            
        case WM_DESTROY:
            pThis->m_shouldClose = true;
            ::PostQuitMessage(0);
            ImGui::SetCurrentContext(prevContext);
            return 0;
        }

        ImGui::SetCurrentContext(prevContext);
    }

    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace UI
