#pragma once
#include <windows.h>
#include <d3d11.h>
#include "imgui.h"

namespace UI {

class BaseWindow {
protected:
    HWND m_hwnd = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
    ImGuiContext* m_imguiContext = nullptr;

    float m_dpiScale = 1.0f;
    bool m_shouldClose = false;
    bool m_swapChainOccluded = false;
    UINT m_resizeWidth = 0;
    UINT m_resizeHeight = 0;

    ImVec4 m_clearColor = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    // Derived classes implement these to configure window settings
    virtual void ConfigureWindow(DWORD& style, DWORD& exStyle, int& width, int& height, const wchar_t*& className, const wchar_t*& windowTitle) = 0;
    virtual void OnInitialize() {}
    virtual void OnDraw() = 0;
    virtual LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { return 0; }

    bool CreateRenderTarget();
    void CleanupRenderTarget();

public:
    BaseWindow();
    virtual ~BaseWindow();

    // Creates the window and D3D11 swap chain
    bool Create();

    // Runs a single step of the window's event processing, updates and renders ImGui
    void RunStep();

    bool ShouldClose() const { return m_shouldClose; }
    HWND GetHwnd() const { return m_hwnd; }

    // Static window procedure mapping messages to BaseWindow instances
    static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

} // namespace UI
