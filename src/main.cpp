#include <windows.h>
#include "ui/main_window.h"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    // Create and initialize the main UI window
    UI::MainWindow mainWindow;
    if (!mainWindow.Create())
    {
        return 1;
    }

    // Show the window
    ::ShowWindow(mainWindow.GetHwnd(), SW_SHOWDEFAULT);
    ::UpdateWindow(mainWindow.GetHwnd());

    // Main event loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
            {
                done = true;
            }
        }
        if (done)
            break;

        if (mainWindow.ShouldClose())
        {
            break;
        }

        // Run updates and render frames for the main window
        mainWindow.RunStep();
    }

    return 0;
}
