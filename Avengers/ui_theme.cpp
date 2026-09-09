#include "pch.h"
#include "ui_theme.h"

static const UiTheme kTheme{};

const UiTheme& uiTheme()
{
	return kTheme;
}

ImU32 uiCol32(const ImVec4& c)
{
	return ImGui::ColorConvertFloat4ToU32(c);
}

void uiThemeApply(ImGuiStyle& style)
{
	const UiTheme& t = kTheme;

	style.WindowRounding = 8.f;
	style.ChildRounding = 6.f;
	style.FrameRounding = 5.f;
	style.GrabRounding = 2.f;
	style.PopupRounding = 6.f;
	style.TabRounding = 6.f;
	style.ScrollbarRounding = 6.f;
	style.WindowPadding = ImVec2(0.f, 0.f);
	style.FramePadding = ImVec2(10.f, 7.f);
	style.ItemSpacing = ImVec2(10.f, 8.f);
	style.ItemInnerSpacing = ImVec2(8.f, 4.f);
	style.ScrollbarSize = 8.f;
	style.WindowBorderSize = 0.f;
	style.ChildBorderSize = 0.f;
	style.FrameBorderSize = 0.f;
	style.PopupBorderSize = 0.f;
	style.GrabMinSize = 4.f;

	style.Colors[ImGuiCol_Text] = t.text;
	style.Colors[ImGuiCol_TextDisabled] = t.textMuted;
	style.Colors[ImGuiCol_WindowBg] = t.bg;
	style.Colors[ImGuiCol_ChildBg] = t.bg;
	style.Colors[ImGuiCol_PopupBg] = t.bgAlt;
	style.Colors[ImGuiCol_Border] = t.border;
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = t.bgAlt;
	style.Colors[ImGuiCol_FrameBgHovered] = t.bgAlt;
	style.Colors[ImGuiCol_FrameBgActive] = t.bgAlt;
	style.Colors[ImGuiCol_TitleBg] = t.bg;
	style.Colors[ImGuiCol_TitleBgActive] = t.bg;
	style.Colors[ImGuiCol_TitleBgCollapsed] = t.bg;
	style.Colors[ImGuiCol_MenuBarBg] = t.bgAlt;
	style.Colors[ImGuiCol_ScrollbarBg] = t.bgAlt;
	style.Colors[ImGuiCol_ScrollbarGrab] = t.surfaceHover;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = t.accent;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = t.accentHover;
	style.Colors[ImGuiCol_CheckMark] = t.accent;
	style.Colors[ImGuiCol_SliderGrab] = t.text;
	style.Colors[ImGuiCol_SliderGrabActive] = t.text;
	style.Colors[ImGuiCol_Button] = t.surface;
	style.Colors[ImGuiCol_ButtonHovered] = t.surfaceHover;
	style.Colors[ImGuiCol_ButtonActive] = t.accentDim;
	style.Colors[ImGuiCol_Header] = t.surfaceHover;
	style.Colors[ImGuiCol_HeaderHovered] = t.surfaceHover;
	style.Colors[ImGuiCol_HeaderActive] = t.accentDim;
	style.Colors[ImGuiCol_Separator] = t.border;
	style.Colors[ImGuiCol_SeparatorHovered] = t.accent;
	style.Colors[ImGuiCol_SeparatorActive] = t.accent;
	style.Colors[ImGuiCol_ResizeGrip] = t.border;
	style.Colors[ImGuiCol_ResizeGripHovered] = t.accent;
	style.Colors[ImGuiCol_ResizeGripActive] = t.accentHover;
	style.Colors[ImGuiCol_Tab] = t.surface;
	style.Colors[ImGuiCol_TabHovered] = t.accent;
	style.Colors[ImGuiCol_TabActive] = t.accent;
	style.Colors[ImGuiCol_TabUnfocused] = t.surface;
	style.Colors[ImGuiCol_TabUnfocusedActive] = t.surface;
	style.Colors[ImGuiCol_PlotLines] = t.accent;
	style.Colors[ImGuiCol_PlotHistogram] = t.accent;
	style.Colors[ImGuiCol_TableHeaderBg] = t.surface;
	style.Colors[ImGuiCol_TableBorderStrong] = t.border;
	style.Colors[ImGuiCol_TableBorderLight] = t.border;
	style.Colors[ImGuiCol_TableRowBg] = t.bg;
	style.Colors[ImGuiCol_TableRowBgAlt] = t.bgAlt;
	style.Colors[ImGuiCol_TextSelectedBg] = t.accentDim;
	style.Colors[ImGuiCol_DragDropTarget] = t.accent;
	style.Colors[ImGuiCol_NavHighlight] = t.accent;
	style.Colors[ImGuiCol_NavWindowingHighlight] = t.accentHover;
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.55f);
}
