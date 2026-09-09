#include "pch.h"
#include "ui_widgets.h"
#include "ui_theme.h"
#include "misc/cpp/imgui_stdlib.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{
	constexpr float kSettingRowGap = 18.f;
	constexpr float kGridGap = 16.f;
	constexpr float kGridMinColumnWidth = 220.f;
	constexpr float kCardPadding = 16.f;
	constexpr float kCardGap = 14.f;

	float gGridColumnWidth = 0.f;
	float gCardMinimumHeight = 0.f;
	bool gTooltipsEnabled = true;

	float lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	ImVec4 lerp(const ImVec4& a, const ImVec4& b, float t)
	{
		return ImVec4(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t));
	}

	float saturate(float v)
	{
		return std::clamp(v, 0.f, 1.f);
	}

	float currentCardWidth()
	{
		const float avail = ImGui::GetContentRegionAvail().x;
		if (gGridColumnWidth > 1.f && gGridColumnWidth < avail + 0.5f) {
			return gGridColumnWidth;
		}
		return avail;
	}

	void finishSetting()
	{
		const ImVec2 p = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(p.x, p.y - ImGui::GetStyle().ItemSpacing.y + kSettingRowGap));
	}

	float rightControlX(float controlW)
	{
		return ImGui::GetCursorScreenPos().x + currentCardWidth() - 16.f - 16.f - controlW;
	}

	// Accepts a single finite number with optional surrounding whitespace; clamps into [minV, maxV].
	bool tryParseBoundedFloat(const char* text, float minV, float maxV, bool integer, float* out)
	{
		char* end = nullptr;
		float value = std::strtof(text, &end);
		if (end == text || !std::isfinite(value)) {
			return false;
		}
		while (std::isspace(static_cast<unsigned char>(*end))) {
			++end;
		}
		if (*end != '\0') {
			return false;
		}
		if (integer) {
			value = std::round(value);
		}
		*out = std::clamp(value, minV, maxV);
		return true;
	}

	bool pillToggle(const char* id, bool* value)
	{
		const UiTheme& t = uiTheme();
		const ImVec2 size(38.f, 20.f);
		const float knob = 14.f;

		ImGui::InvisibleButton(id, size);
		const bool pressed = ImGui::IsItemClicked();
		if (pressed) {
			*value = !*value;
		}

		const ImGuiID itemId = ImGui::GetItemID();
		ImGuiStorage* storage = ImGui::GetStateStorage();
		float target = 0.f;
		if (*value) {
			target = 1.f;
		}
		float anim = storage->GetFloat(itemId, target);
		anim = lerp(anim, target, std::fmin(ImGui::GetIO().DeltaTime * 14.f, 1.f));
		if (anim > 0.995f && target == 1.f) anim = 1.f;
		if (anim < 0.005f && target == 0.f) anim = 0.f;
		storage->SetFloat(itemId, anim);

		const ImVec2 pmin = ImGui::GetItemRectMin();
		const ImVec2 pmax = ImGui::GetItemRectMax();
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pmin, pmax, uiCol32(lerp(t.toggleOff, t.accent, anim)), 10.f);

		const float pad = (size.y - knob) * 0.5f;
		const float knobX = lerp(pmin.x + pad, pmax.x - pad - knob, anim);
		const float knobY = pmin.y + pad;
		draw->AddCircleFilled(ImVec2(knobX + knob * 0.5f, knobY + knob * 0.5f), knob * 0.5f, IM_COL32(255, 255, 255, 255));
		return pressed;
	}

	bool thinSlider(const char* id, float* value, float minV, float maxV)
	{
		const UiTheme& t = uiTheme();
		const float width = currentCardWidth() - 32.f;
		ImGui::InvisibleButton(id, ImVec2(width, 16.f));
		const float previousValue = *value;
		const bool active = ImGui::IsItemActive();
		if (active && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			const ImVec2 pmin = ImGui::GetItemRectMin();
			const ImVec2 pmax = ImGui::GetItemRectMax();
			const float t01 = saturate((ImGui::GetIO().MousePos.x - pmin.x) / (pmax.x - pmin.x));
			*value = minV + t01 * (maxV - minV);
		}

		const ImVec2 pmin = ImGui::GetItemRectMin();
		const ImVec2 pmax = ImGui::GetItemRectMax();
		const float midY = (pmin.y + pmax.y) * 0.5f;
		float span = maxV - minV;
		if (span <= 0.f) {
			span = 1.f;
		}
		const float t01 = saturate((*value - minV) / span);
		const float knobX = pmin.x + t01 * (pmax.x - pmin.x);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddLine(ImVec2(pmin.x, midY), ImVec2(pmax.x, midY), uiCol32(t.toggleOff), 2.f);
		draw->AddLine(ImVec2(pmin.x, midY), ImVec2(knobX, midY), uiCol32(t.accent), 2.f);
		draw->AddRectFilled(ImVec2(knobX - 1.f, midY - 6.f), ImVec2(knobX + 1.f, midY + 6.f), uiCol32(t.text), 1.f);
		return *value != previousValue;
	}

	bool settingTextInputImpl(const std::string& label, const std::string& desc, const auto& drawInput)
	{
		ImGui::PushID(label.c_str());
		ImGui::TextUnformatted(label.c_str());
		ImGui::SetNextItemWidth(currentCardWidth() - 32.f);
		const bool changed = drawInput();
		uiw::tooltip(desc);
		finishSetting();
		ImGui::PopID();
		return changed;
	}
}

namespace uiw
{
	void setTooltipsEnabled(bool enabled)
	{
		gTooltipsEnabled = enabled;
	}

	void tooltip(const std::string& text)
	{
		if (gTooltipsEnabled && !text.empty()) {
			ImGui::SetItemTooltip("%s", text.c_str());
		}
	}

	void beginGrid()
	{
		gGridColumnWidth = (ImGui::GetContentRegionAvail().x - kGridGap) * 0.5f;
		if (gGridColumnWidth < kGridMinColumnWidth) {
			gGridColumnWidth = 0.f;
			return;
		}
		ImGui::BeginGroup();
	}

	void nextGridColumn()
	{
		if (gGridColumnWidth <= 0.f) {
			return;
		}
		ImGui::EndGroup();
		ImGui::SameLine(0.f, kGridGap);
		ImGui::BeginGroup();
	}

	void endGrid()
	{
		if (gGridColumnWidth > 0.f) {
			ImGui::EndGroup();
		}
		gGridColumnWidth = 0.f;
	}

	void beginCard(const std::string& id, float minimumHeight)
	{
		ImGui::PushID(id.c_str());
		gCardMinimumHeight = minimumHeight;
		if (minimumHeight > 0.f) {
			ImGui::BeginGroup();
		}

		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->ChannelsSplit(2);
		draw->ChannelsSetCurrent(1);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(currentCardWidth(), kCardPadding));
		ImGui::Indent(kCardPadding);
	}

	void endCard()
	{
		const UiTheme& t = uiTheme();
		ImGui::Unindent(kCardPadding);
		ImGui::Dummy(ImVec2(0.f, kCardPadding));
		ImGui::EndGroup();

		ImVec2 rmin = ImGui::GetItemRectMin();
		ImVec2 rmax = ImGui::GetItemRectMax();
		if (gCardMinimumHeight > 0.f) {
			const float fillerHeight = gCardMinimumHeight - (rmax.y - rmin.y) - ImGui::GetStyle().ItemSpacing.y;
			if (fillerHeight > 0.f) {
				ImGui::Dummy(ImVec2(0.f, fillerHeight));
			}
			ImGui::EndGroup();
			rmin = ImGui::GetItemRectMin();
			rmax = ImGui::GetItemRectMax();
		}

		const float width = currentCardWidth();
		if (width > 1.f) {
			rmax.x = rmin.x + width;
		}

		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->ChannelsSetCurrent(0);
		draw->AddRectFilled(rmin, rmax, uiCol32(t.surface), 6.f);
		draw->ChannelsMerge();

		ImGui::Dummy(ImVec2(0.f, kCardGap));
		gCardMinimumHeight = 0.f;
		ImGui::PopID();
	}

	bool settingToggle(const std::string& label, const std::string& desc, bool* value)
	{
		ImGui::PushID(label.c_str());
		const float toggleW = 38.f;
		const float wrapW = currentCardWidth() - toggleW - 40.f;

		ImGui::BeginGroup();
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapW);
		ImGui::TextUnformatted(label.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndGroup();
		tooltip(desc);

		const ImVec2 labelMin = ImGui::GetItemRectMin();
		const float rowH = ImGui::GetItemRectMax().y - labelMin.y;
		const float toggleY = labelMin.y + std::fmax(0.f, (rowH - 20.f) * 0.5f);

		const ImVec2 afterLabel = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(rightControlX(toggleW), toggleY));
		const bool changed = pillToggle("##toggle", value);
		tooltip(desc);
		const float rowBottom = std::fmax(afterLabel.y, ImGui::GetItemRectMax().y);
		ImGui::SetCursorScreenPos(ImVec2(afterLabel.x, rowBottom + kSettingRowGap));
		ImGui::PopID();
		return changed;
	}

	bool settingTextInput(const std::string& label, const std::string& desc, std::string* value, const std::string& hint)
	{
		return settingTextInputImpl(label, desc, [&] {
			if (hint.empty()) {
				return ImGui::InputText("##input", value);
			}
			return ImGui::InputTextWithHint("##input", hint.c_str(), value);
		});
	}

	bool settingSlider(const std::string& label, const std::string& desc, float* value, float minV, float maxV, const std::string& fmt)
	{
		ImGui::PushID(label.c_str());
		const bool integer = (fmt == "%.0f");
		ImGuiStorage* storage = ImGui::GetStateStorage();
		const ImGuiID editingId = ImGui::GetID("##editing");
		const ImGuiID focusId = ImGui::GetID("##focus");
		bool editing = storage->GetBool(editingId, false);

		struct SliderEditBuffer
		{
			ImGuiID owner = 0;
			char text[32]{};
		};
		static SliderEditBuffer edit;

		char valueText[32];
		std::snprintf(valueText, sizeof(valueText), fmt.c_str(), *value);
		const char* displayedValue = valueText;
		if (editing) {
			displayedValue = edit.text;
		}
		const ImVec2 valueTextSize = ImGui::CalcTextSize(displayedValue);
		const float valueFieldW = std::fmax(valueTextSize.x + 16.f, 56.f);
		const float valueFieldH = ImGui::GetTextLineHeight() + 6.f;

		ImGui::TextUnformatted(label.c_str());
		const ImVec2 afterLabel = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(
			rightControlX(valueFieldW),
			afterLabel.y - ImGui::GetTextLineHeight() - ImGui::GetStyle().ItemSpacing.y - 2.f));

		bool changed = false;
		if (editing && edit.owner == editingId) {
			if (storage->GetBool(focusId, false)) {
				ImGui::SetKeyboardFocusHere();
				storage->SetBool(focusId, false);
			}
			ImGui::SetNextItemWidth(valueFieldW);
			ImGui::InputText("##value", edit.text, sizeof(edit.text),
				ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue);
			tooltip("Enter a number. Invalid input is ignored; values are clamped to the slider range.");

			if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemDeactivated()) {
				float parsed = *value;
				if (tryParseBoundedFloat(edit.text, minV, maxV, integer, &parsed) && parsed != *value) {
					*value = parsed;
					changed = true;
				}
				storage->SetBool(editingId, false);
				editing = false;
				edit.owner = 0;
			}
		}
		else {
			const ImVec2 fieldMin = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("##valueHit", ImVec2(valueFieldW, valueFieldH));
			const bool hovered = ImGui::IsItemHovered();
			if (ImGui::IsItemClicked()) {
				edit.owner = editingId;
				std::snprintf(edit.text, sizeof(edit.text), fmt.c_str(), *value);
				storage->SetBool(editingId, true);
				storage->SetBool(focusId, true);
			}

			const ImVec2 fieldMax = ImGui::GetItemRectMax();
			ImDrawList* draw = ImGui::GetWindowDrawList();
			ImVec4 fieldBg = uiTheme().bgAlt;
			if (hovered) {
				fieldBg = uiTheme().surfaceHover;
			}
			draw->AddRectFilled(fieldMin, fieldMax, uiCol32(fieldBg), 4.f);
			draw->AddRect(fieldMin, fieldMax, uiCol32(uiTheme().border), 4.f);
			const ImVec2 textPos(
				fieldMin.x + (valueFieldW - valueTextSize.x) * 0.5f,
				fieldMin.y + (valueFieldH - valueTextSize.y) * 0.5f);
			draw->AddText(textPos, uiCol32(uiTheme().textMuted), valueText);
			tooltip("Click to type a value.");
		}

		ImGui::SetCursorScreenPos(afterLabel);
		if (thinSlider("##slider", value, minV, maxV)) {
			changed = true;
			if (editing) {
				storage->SetBool(editingId, false);
				edit.owner = 0;
			}
		}
		tooltip(desc);
		finishSetting();
		ImGui::PopID();
		return changed;
	}

	bool settingColor(const std::string& label, const std::string& desc, ImVec4* color)
	{
		ImGui::PushID(label.c_str());
		ImGui::TextUnformatted(label.c_str());

		const ImVec2 afterLabel = ImGui::GetCursorScreenPos();
		const float swatch = 16.f;
		float swatchY = afterLabel.y - ImGui::GetTextLineHeight() - ImGui::GetStyle().ItemSpacing.y;
		ImGui::SetCursorScreenPos(ImVec2(rightControlX(swatch), swatchY));
		const bool opened = ImGui::ColorButton("##swatch", *color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview, ImVec2(swatch, swatch));
		tooltip(desc);
		if (opened) {
			ImGui::OpenPopup("##colorPopup");
		}
		ImGui::SetCursorScreenPos(afterLabel);

		bool changed = false;
		if (ImGui::BeginPopup("##colorPopup")) {
			changed = ImGui::ColorPicker4("##picker", &color->x, ImGuiColorEditFlags_AlphaBar);
			ImGui::EndPopup();
		}
		ImGui::SetCursorScreenPos(ImVec2(afterLabel.x, afterLabel.y + kSettingRowGap));
		ImGui::PopID();
		return changed;
	}

	bool settingCombo(const std::string& label, const std::string& desc, std::string* current, const std::vector<std::string>& items)
	{
		ImGui::PushID(label.c_str());
		ImGui::TextUnformatted(label.c_str());
		ImGui::SetNextItemWidth(currentCardWidth() - 32.f);
		bool changed = false;
		if (ImGui::BeginCombo("##combo", current->c_str())) {
			for (const std::string& item : items) {
				const bool selected = (*current == item);
				if (ImGui::Selectable(item.c_str(), selected)) {
					*current = item;
					changed = true;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		tooltip(desc);
		finishSetting();
		ImGui::PopID();
		return changed;
	}

	bool navButton(const std::string& label, bool active)
	{
		const UiTheme& t = uiTheme();
		const ImVec2 size(ImGui::GetContentRegionAvail().x, 40.f);
		ImGui::InvisibleButton(label.c_str(), size);
		const bool pressed = ImGui::IsItemClicked();
		const bool hovered = ImGui::IsItemHovered();

		const ImVec2 pmin = ImGui::GetItemRectMin();
		const ImVec2 pmax = ImGui::GetItemRectMax();
		ImDrawList* draw = ImGui::GetWindowDrawList();

		if (active || hovered) {
			ImVec4 fill = t.surface;
			if (active) {
				fill = t.surfaceHover;
			}
			draw->AddRectFilled(pmin, pmax, uiCol32(fill), 8.f);
		}

		const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
		const ImVec2 textPos(pmin.x + 12.f, pmin.y + (size.y - textSize.y) * 0.5f);
		ImVec4 textColor = t.text;
		if (active) {
			textColor = t.accent;
		}
		draw->AddText(textPos, uiCol32(textColor), label.c_str());
		return pressed;
	}

	bool accentButton(const std::string& label, const ImVec2& size)
	{
		const UiTheme& t = uiTheme();
		ImGui::PushStyleColor(ImGuiCol_Button, t.accent);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, t.accentHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, t.accent);
		ImGui::PushStyleColor(ImGuiCol_Text, t.text);
		const bool pressed = ImGui::Button(label.c_str(), size);
		ImGui::PopStyleColor(4);
		return pressed;
	}
}
