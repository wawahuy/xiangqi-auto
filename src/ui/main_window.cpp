#include "main_window.h"
#include "imgui.h"

namespace UI {

void MainWindow::ConfigureWindow(DWORD& style, DWORD& exStyle, int& width, int& height, const wchar_t*& className, const wchar_t*& windowTitle)
{
    style = WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU;
    exStyle = 0;
    width = 800;
    height = 600;
    className = L"XiangqiMainWindowClass";
    windowTitle = L"Xiangqi Auto";
}

void MainWindow::OnDraw()
{
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("MainWindowBackdrop", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus
    );

    float current_width = io.DisplaySize.x;
    float current_height = io.DisplaySize.y;

    // Custom Title Bar Area
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 title_bar_min = ImGui::GetWindowPos();
    ImVec2 title_bar_max = ImVec2(title_bar_min.x + current_width, title_bar_min.y + 40.0f);
    draw_list->AddRectFilled(title_bar_min, title_bar_max, IM_COL32(26, 26, 28, 255));

    // Custom dragging logic when clicking the title bar
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 window_pos = ImGui::GetWindowPos();
        float local_x = mouse_pos.x - window_pos.x;
        float local_y = mouse_pos.y - window_pos.y;

        // Only drag if mouse is in the title bar area and not on control buttons (which are on the right)
        if (local_y >= 0.0f && local_y <= 40.0f && local_x >= 0.0f && local_x < (current_width - 90.0f))
        {
            ::ReleaseCapture();
            ::SendMessageW(m_hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        }
    }

    // Title Bar Label
    ImGui::SetCursorPos(ImVec2(15.0f, 12.0f));
    ImGui::Text("Xiangqi Auto");

    // Control buttons (Minimize and Close) styling
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent default background
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.35f, 0.6f));

    // Minimize Button
    ImGui::SetCursorPos(ImVec2(current_width - 90.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.24f, 1.0f));
    bool minimize_clicked = ImGui::Button("##Minimize", ImVec2(45.0f, 40.0f));
    ImGui::PopStyleColor();

    // Draw minimize vector icon
    ImVec2 min_btn_pos = ImGui::GetItemRectMin();
    ImVec2 min_btn_size = ImGui::GetItemRectSize();
    bool min_hovered = ImGui::IsItemHovered();
    ImU32 min_color = min_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 200, 200, 255);
    draw_list->AddLine(
        ImVec2(min_btn_pos.x + min_btn_size.x * 0.38f, min_btn_pos.y + min_btn_size.y * 0.55f),
        ImVec2(min_btn_pos.x + min_btn_size.x * 0.62f, min_btn_pos.y + min_btn_size.y * 0.55f),
        min_color,
        1.5f
    );

    if (minimize_clicked)
    {
        ::ShowWindow(m_hwnd, SW_MINIMIZE);
    }

    // Close Button
    ImGui::SetCursorPos(ImVec2(current_width - 45.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.15f, 0.15f, 1.0f)); // Premium red hover
    bool close_clicked = ImGui::Button("##Close", ImVec2(45.0f, 40.0f));
    ImGui::PopStyleColor();

    // Draw close vector icon (X)
    ImVec2 cls_btn_pos = ImGui::GetItemRectMin();
    ImVec2 cls_btn_size = ImGui::GetItemRectSize();
    bool cls_hovered = ImGui::IsItemHovered();
    ImU32 cls_color = cls_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 200, 200, 255);
    float pad_ratio_min = 0.38f;
    float pad_ratio_max = 0.62f;
    draw_list->AddLine(
        ImVec2(cls_btn_pos.x + cls_btn_size.x * pad_ratio_min, cls_btn_pos.y + cls_btn_size.y * pad_ratio_min),
        ImVec2(cls_btn_pos.x + cls_btn_size.x * pad_ratio_max, cls_btn_pos.y + cls_btn_size.y * pad_ratio_max),
        cls_color,
        1.5f
    );
    draw_list->AddLine(
        ImVec2(cls_btn_pos.x + cls_btn_size.x * pad_ratio_max, cls_btn_pos.y + cls_btn_size.y * pad_ratio_min),
        ImVec2(cls_btn_pos.x + cls_btn_size.x * pad_ratio_min, cls_btn_pos.y + cls_btn_size.y * pad_ratio_max),
        cls_color,
        1.5f
    );

    if (close_clicked)
    {
        m_shouldClose = true;
    }

    // Restore button colors
    ImGui::PopStyleColor(2);

    // Content Area
    ImGui::SetCursorPos(ImVec2(20.0f, 60.0f));
    ImGui::BeginChild("ContentArea", ImVec2(current_width - 40.0f, current_height - 80.0f), false, ImGuiWindowFlags_None);

    ImGui::Text("Dear ImGui integration completed successfully!");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Environment Information:");
    ImGui::BulletText("Graphics Backend: DirectX 11");
    ImGui::BulletText("Window Style: Borderless Window (WS_POPUP)");
    ImGui::BulletText("Shadow System: CS_DROPSHADOW Enabled");
    ImGui::BulletText("DPI Scale: %.2f", m_dpiScale);
    ImGui::Spacing();

    static bool show_demo_window = false;
    if (ImGui::Button(show_demo_window ? "Hide Demo Window" : "Show Demo Window", ImVec2(180.0f, 30.0f)))
    {
        show_demo_window = !show_demo_window;
    }

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar(3);
}

} // namespace UI
