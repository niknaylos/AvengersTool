#pragma once
#include <string>
#include <vector>
#include "imgui.h"

namespace uiw
{
	void beginGrid();
	void nextGridColumn();
	void endGrid();
	void beginCard(const std::string& id, float minimumHeight = 0.f);
	void endCard();
	bool settingToggle(const std::string& label, const std::string& desc, bool* value);
	bool settingSlider(const std::string& label, const std::string& desc, float* value, float minV, float maxV, const std::string& fmt = "%.2f");
	bool settingColor(const std::string& label, const std::string& desc, ImVec4* color);
	bool settingCombo(const std::string& label, const std::string& desc, std::string* current, const std::vector<std::string>& items);
	bool settingTextInput(const std::string& label, const std::string& desc, std::string* value, const std::string& hint = "");
	bool navButton(const std::string& label, bool active);
	bool accentButton(const std::string& label, const ImVec2& size = ImVec2(0.f, 0.f));
	void tooltip(const std::string& text);
	void setTooltipsEnabled(bool enabled);
}
