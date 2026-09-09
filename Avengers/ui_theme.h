#pragma once
#include "imgui.h"

struct UiTheme
{
	ImVec4 bg            { 0.039f, 0.039f, 0.039f, 0.98f };
	ImVec4 bgAlt         { 0.020f, 0.020f, 0.020f, 1.00f };
	ImVec4 surface       { 0.071f, 0.071f, 0.071f, 1.00f };
	ImVec4 surfaceHover  { 0.110f, 0.110f, 0.110f, 1.00f };
	ImVec4 border        { 0.120f, 0.120f, 0.120f, 1.00f };
	ImVec4 text          { 1.000f, 1.000f, 1.000f, 1.00f };
	ImVec4 textMuted     { 0.557f, 0.557f, 0.557f, 1.00f };
	ImVec4 accent        { 0.031f, 0.835f, 0.729f, 1.00f };
	ImVec4 accentHover   { 0.180f, 0.910f, 0.816f, 1.00f };
	ImVec4 accentDim     { 0.031f, 0.835f, 0.729f, 0.18f };
	ImVec4 success       { 0.239f, 0.863f, 0.592f, 1.00f };
	ImVec4 warning       { 0.961f, 0.647f, 0.141f, 1.00f };
	ImVec4 danger        { 0.941f, 0.443f, 0.471f, 1.00f };
	ImVec4 toggleOff     { 0.137f, 0.118f, 0.090f, 1.00f };
};

const UiTheme& uiTheme();
ImU32 uiCol32(const ImVec4& c);
void uiThemeApply(ImGuiStyle& style);
