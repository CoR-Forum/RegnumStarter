#include "includes/Utils.h"

extern ImVec4 textColor;

void ApplyCustomStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Apply the loaded text color
    style.Colors[ImGuiCol_Text] = textColor;

    style.WindowBorderSize = 2.0f; // Border size of the window.
    style.WindowPadding = ImVec2(15, 15); // Padding inside a window.
    style.WindowRounding = 10.0f; // Rounding of window corners.
    style.FramePadding = ImVec2(5, 5); // Padding inside a frame.
    style.FrameRounding = 4.0f; // Rounding of frame corners.
    style.ItemSpacing = ImVec2(12, 8); // Spacing between items.
    style.ItemInnerSpacing = ImVec2(8, 6); //Spacing between elements inside a composite item.
    style.IndentSpacing = 25.0f; // Indentation spacing.
    style.ScrollbarSize = 15.0f; // Size of the scrollbar.
    style.ScrollbarRounding = 9.0f; // Rounding of scrollbar corners.
    style.GrabMinSize = 5.0f; // Minimum size of the grab area.
    style.GrabRounding = 3.0f; // Rounding of grab corners.

    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f); // Color for disabled text.
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f); // Background color of the window.
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f); // Background color of popups.
    style.Colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // Color of borders.
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Shadow color of borders.
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); // Background color of frames.
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f); // Background color of frames when hovered.
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f); // Background color of frames when active.
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); // Background color of the title bar.
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f); // Background color of the title bar when collapsed.
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f); //  Background color of the title bar when active.
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); // Background color of the menu bar.
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); // Background color of the scrollbar.
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f); // Color of the scrollbar grab area.
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f); // Color of the scrollbar grab area when hovered.
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f); // Color of the scrollbar grab area when active.
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f); // Color of the check mark.
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f); // Color of the slider grab area.
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f); // Color of the slider grab area when active.
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); // Color of buttons.
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f); // Color of buttons when hovered.
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f); // Color of buttons when active.
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f); // Color of headers.
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f); // Color of headers when hovered.
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f); // Color of headers when active.
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Color of the resize grip.

    // Load and set the font from resource
    ImGuiIO& io = ImGui::GetIO();
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_FONT_RUDA_BOLD), RT_FONT);
    if (hResource)
    {
        HGLOBAL hMemory = LoadResource(NULL, hResource);
        if (hMemory)
        {
            void* pData = LockResource(hMemory);
            DWORD size = SizeofResource(NULL, hResource);
            if (pData && size > 0)
            {
                io.Fonts->AddFontFromMemoryTTF(pData, size, 13.0f);
                io.FontDefault = io.Fonts->Fonts.back();
            }
        }
    }
}