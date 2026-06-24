#pragma once
#include "base_window.h"

namespace UI {

class MainWindow : public BaseWindow {
protected:
    // Configures the borderless main window
    void ConfigureWindow(DWORD& style, DWORD& exStyle, int& width, int& height, const wchar_t*& className, const wchar_t*& windowTitle) override;

    // Renders the main window ImGui controls
    void OnDraw() override;
};

} // namespace UI
