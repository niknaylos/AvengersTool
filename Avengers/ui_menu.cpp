#include "pch.h"
#include "ui_menu.h"
#include "Avengers.h"
#include "ui_theme.h"
#include "ui_widgets.h"
#include <algorithm>
#include <array>
#include <format>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

const std::array<ui_menu::MenuTabInfo, 9> ui_menu::kMenuTabs = {
	MenuTabInfo{ "General", "Display, fonts, and interface behavior.", &ui_menu::drawGeneral },
	MenuTabInfo{ "Velocity", "Speedometer layout, colors, and accel cues.", &ui_menu::drawVelocity },
	MenuTabInfo{ "Jump Target", "Target placement and brush selection.", &ui_menu::drawJumpTarget },
	MenuTabInfo{ "Angle Helper", "Strafe zones, centerline, and indicator styling.", &ui_menu::drawAngleHelper },
	MenuTabInfo{ "FPS Wheel", "FPS zone wheel layout, scaling, and centerline.", &ui_menu::drawFpsWheel },
	MenuTabInfo{ "Misc", "Timers, position tools, and extra HUD readouts.", &ui_menu::drawMisc },
	MenuTabInfo{ "Collision", "World clip visualization.", &ui_menu::drawCollisionPage },
	MenuTabInfo{ "Markers", "On-map markers and positioning help.", &ui_menu::drawMarkers },
	MenuTabInfo{ "Demo Player", "Demo playback and batch controls.", &ui_menu::drawDemoPlayer },
};

namespace
{
	std::string ellipsizeText(const std::string& text, float availableWidth)
	{
		if (ImGui::CalcTextSize(text.c_str()).x <= availableWidth) {
			return text;
		}

		const std::string ellipsis = "...";
		const float ellipsisWidth = ImGui::CalcTextSize(ellipsis.c_str()).x;
		if (availableWidth <= ellipsisWidth) {
			return ellipsis;
		}

		std::string result = text;
		while (!result.empty() && ImGui::CalcTextSize(result.c_str()).x + ellipsisWidth > availableWidth) {
			result.pop_back();
		}
		return result + ellipsis;
	}

	void dragWindowIfHeld()
	{
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			const ImVec2 pos = ImGui::GetWindowPos();
			ImGui::SetWindowPos(ImVec2(pos.x + ImGui::GetIO().MouseDelta.x, pos.y + ImGui::GetIO().MouseDelta.y));
		}
	}

	void drawSidebarDragArea()
	{
		const float available = ImGui::GetContentRegionAvail().y;
		if (available > 0.f) {
			ImGui::InvisibleButton("##sidebarDrag", ImVec2(-1.f, available));
			dragWindowIfHeld();
		}
	}

	void saveIf(Avengers* hud, bool changed)
	{
		if (changed) {
			hud->saveConfiguration();
		}
	}

	void readonlyVec3(const std::string& label, const vec3<float>& value)
	{
		ImGui::TextUnformatted(label.c_str());
		ImGui::TextDisabled("%.4f   %.4f   %.4f", value.x, value.y, value.z);
		ImGui::Dummy(ImVec2(0.f, 10.f));
	}
}

bool ui_menu::fontCombo(const std::string& label, const std::string& desc, std::string* selected, const std::string& fallback)
{
	if (loadedFonts.empty()) {
		return false;
	}

	std::vector<std::string> names;
	names.reserve(loadedFonts.size());
	for (const auto& entry : loadedFonts) {
		names.push_back(entry.first);
	}
	std::sort(names.begin(), names.end());

	bool corrected = false;
	if (!loadedFonts.contains(*selected)) {
		if (loadedFonts.contains(fallback)) {
			*selected = fallback;
		}
		else {
			*selected = names.front();
		}
		corrected = true;
	}

	return uiw::settingCombo(label, desc, selected, names) || corrected;
}

void ui_menu::drawGeneral(Avengers* hud)
{
	constexpr float kGeneralCardHeight = 270.f;
	uiw::beginGrid();
	uiw::beginCard("Display", kGeneralCardHeight);
	saveIf(hud, uiw::settingToggle("Show coordinates", "XYZ and view angles while connected.", &showPosition));
	saveIf(hud, uiw::settingToggle("Show tooltips", "Hover descriptions on menu settings.", &showMenuTooltips));
	saveIf(hud, uiw::settingToggle("Allow impure map IWDs", "Modified IWDs in usermaps will not force a server redownload.", &allowImpureMapIwds));
	// WIP overlay — restore these in Display when the FPS readout is implemented.
	// saveIf(hud, uiw::settingToggle("Draw FPS", "On-screen FPS readout (WIP overlay).", &drawfpsToggle));
	// saveIf(hud, uiw::settingToggle("FPS only while spectating", "Hide the FPS overlay unless you are spectating.", &drawfpsSpectateonly));
	// saveIf(hud, uiw::settingSlider("FPS scale", "", &fpsScale, 0.4f, 3.f));
	// saveIf(hud, uiw::settingColor("FPS color", "", &fpsColor));
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("ImGui font", kGeneralCardHeight);
	if (fontCombo("Menu font", "Used by this menu.", &selectedMenuFont, "Trebuchet")) {
		hud->saveConfiguration();
	}
	saveIf(hud, uiw::settingSlider("Menu font size", "Pixel size of this menu.", &menuFontSize, 10.f, 32.f, "%.0f"));
	if (fontCombo("Speedometer font", "Used by the in-game speedometer.", &selectedSpeedometerFont, "Bahnschrift")) {
		hud->saveConfiguration();
	}
	uiw::endCard();
	uiw::endGrid();
}

void ui_menu::drawVelocity(Avengers* hud)
{
	uiw::beginGrid();
	uiw::beginCard("Speedometer");
	saveIf(hud, uiw::settingToggle("Speedometer", "Show horizontal speed on the HUD.", &veloMeter));
	saveIf(hud, uiw::settingSlider("Speed size", "", &veloScale, 0.01f, 10.f));
	saveIf(hud, uiw::settingToggle("Lock speed position", "Prevent dragging the speedometer.", &lockVeloPos));
	saveIf(hud, uiw::settingToggle("Keep centered", "Keeps the speedometer centered on the screen.", &keepVeloCentered));
	saveIf(hud, uiw::settingToggle("Use static positioning", "Prevent text from moving on velo change.", &useStaticPositioning));
	saveIf(hud, uiw::settingColor("Speed color", "", &color));
	if (uiw::accentButton("Center")) {
		centerSpeedometer(hud);
	}
	uiw::tooltip("Centers the speedometer on the screen");
	uiw::endCard();

	uiw::beginCard("Takeoff");
	saveIf(hud, uiw::settingToggle("Jumpoff speed", "Displays jumpoff speed below the speedometer.", &drawJumpoffSpeed));
	saveIf(hud, uiw::settingToggle("Display bottom left", "Also print jumpoff speed as an obituary.", &jumpoffspeedDisplayBottom));
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("Acceleration");
	saveIf(hud, uiw::settingToggle("Show acceleration", "Recolor the speedometer while speeding up.", &veloShowAcceleration));
	saveIf(hud, uiw::settingToggle("Enable acceleration on ground", "", &enableAccelerationOnGround));
	saveIf(hud, uiw::settingSlider("Acceleration threshold", "Frames to wait before confirming accel.", &veloAccelerationThreshold, 1.f, 100.f, "%.0f"));
	saveIf(hud, uiw::settingSlider("Frames to keep acceleration", "", &veloKeepAccelFor, 0.f, 100.f, "%.0f"));
	saveIf(hud, uiw::settingColor("Acceleration color", "", &accelerationColor));
	uiw::endCard();

	uiw::beginCard("Deceleration");
	saveIf(hud, uiw::settingToggle("Show deceleration", "Recolor the speedometer while slowing down.", &veloShowDeceleration));
	saveIf(hud, uiw::settingToggle("Enable deceleration on ground", "", &enableDecelerationOnGround));
	saveIf(hud, uiw::settingSlider("Deceleration threshold", "Frames to wait before confirming decel.", &veloDecelerationThreshold, 1.f, 100.f, "%.0f"));
	saveIf(hud, uiw::settingSlider("Frames to keep deceleration", "", &veloKeepDecelFor, 0.f, 100.f, "%.0f"));
	saveIf(hud, uiw::settingColor("Deceleration color", "", &decelerationColor));
	uiw::endCard();
	uiw::endGrid();
}

void ui_menu::drawJumpTarget(Avengers* hud)
{
	std::string brushModeDescription = "Draw collision must process brushes once per map before brush mode can be used.";
	if (hud->collision->hasInitialized) {
		brushModeDescription = "Select collision brushes as the jump target.";
	}

	uiw::beginCard("Jump target");
	saveIf(hud, uiw::settingToggle("Enable jump target", "Measure distance to a saved origin or selected brushes.", &jumpTarget));
	saveIf(hud, uiw::settingToggle("Select closest", "Add brush selects the closest brush instead of one at the same height.", &jumpTargetSelectClosest));
	saveIf(hud, uiw::settingToggle("Brush mode", brushModeDescription, &brushMode));
	saveIf(hud, uiw::settingToggle("Draw selected brushes", "", &drawSelectedBrushes));
	readonlyVec3("Jump target origin", jumpTargetOrigin);

	ImGui::Dummy(ImVec2(0.f, 4.f));
	if (!brushMode || !hud->collision->hasInitialized) {
		if (uiw::accentButton("Set jump target")) {
			jumpTargetOrigin = hud->instGame->getOrigin();
		}
	}
	else {
		if (uiw::accentButton("Add brush")) {
			hud->instUiJumpTarget->addBrush();
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove brush")) {
			hud->instUiJumpTarget->removeBrush();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {
			hud->instUiJumpTarget->resetBrushes();
		}
	}
	uiw::endCard();
}

void ui_menu::drawAngleHelper(Avengers* hud)
{
	static const std::vector<std::string> kAhStyles = { "Style 1", "Style 2" };
	if (currentAhStyle != "Style 1" && currentAhStyle != "Style 2") {
		currentAhStyle = "Style 2";
		hud->saveConfiguration();
	}

	uiw::beginGrid();
	uiw::beginCard("Angle helper");
	saveIf(hud, uiw::settingToggle("Angle helper", "Draw the optimal-angle indicator.", &anglehelperToggle));
	saveIf(hud, uiw::settingCombo("Style", "", &currentAhStyle, kAhStyles));
	saveIf(hud, uiw::settingToggle("Clamp to next zone", "Style 2 only, clamps anglehelper rectangle to the next fps zone.", &clampToNextZone));
	saveIf(hud, uiw::settingSlider("Y offset", "", &anglehelperYOffset, -1000.f, 1000.f, "%.0f"));
	saveIf(hud, uiw::settingSlider("Width", "", &anglehelperWidth, 1.f, 10.f));
	saveIf(hud, uiw::settingSlider("Height", "", &anglehelperHeight, 1.f, 3.f));
	saveIf(hud, uiw::settingSlider("Pixel scale", "Stretches or squishes the anglehelper.", &ahPixelScale, 0.3f, 1.f));
	saveIf(hud, uiw::settingColor("Color", "", &anglehelperColor));
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("Centerline");
	saveIf(hud, uiw::settingToggle("Draw centerline", "", &drawcenterline));
	saveIf(hud, uiw::settingSlider("Centerline width", "", &centerlineWidth, 1.f, 10.f));
	saveIf(hud, uiw::settingColor("Centerline color", "", &centerlineColor));
	uiw::endCard();
	uiw::endGrid();
}

void ui_menu::drawFpsWheel(Avengers* hud)
{
	constexpr float kFpsWheelCardHeight = 330.f;
	uiw::beginGrid();
	uiw::beginCard("FPS wheel", kFpsWheelCardHeight);
	if (uiw::settingToggle("FPS wheel", "", &fpswheelToggle)) {
		hud->saveConfiguration();
		if (fpswheelToggle) {
			shouldFocusNextFrame = true;
		}
	}
	saveIf(hud, uiw::settingSlider("Height", "", &fpswheelSize, 1.f, 100.f, "%.0f"));
	saveIf(hud, uiw::settingSlider("X span", "Fraction of screen width the wheel covers.", &fpswheelOffsetX, 0.45f, 1.f));
	saveIf(hud, uiw::settingSlider("Y offset", "", &fpswheelOffsetY, -200.f, 200.f, "%.0f"));
	saveIf(hud, uiw::settingSlider("Wheel pixel scale", "Stretches or squishes the wheel.", &wheelPixelScale, 0.3f, 1.f));
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("Wheel indicators", kFpsWheelCardHeight);
	saveIf(hud, uiw::settingSlider("Wheel anglehelper pixel scale", "Stretches or squishes the wheel anglehelper.", &wheelAhPixelScale, 0.3f, 1.f));
	saveIf(hud, uiw::settingToggle("Draw wheel centerline", "", &drawfpswheelcenterline));
	saveIf(hud, uiw::settingColor("Wheel centerline color", "", &fpswheelcenterlineColor));
	uiw::endCard();
	uiw::endGrid();
}

void ui_menu::drawMisc(Avengers* hud)
{
	constexpr float kMiscCardHeight = 340.f;
	uiw::beginGrid();
	uiw::beginCard("Readouts", kMiscCardHeight);
	saveIf(hud, uiw::settingToggle("90 lines", "World-space 90-degree guide lines.", &linesToggle));
	saveIf(hud, uiw::settingColor("90 lines color", "", &linesColor));
	saveIf(hud, uiw::settingToggle("Strafe downtime", "Measures acceleration downtime when switching strafes.", &strafedowntimeToggle));
	saveIf(hud, uiw::settingToggle("RPG timer", "Displays frames passed between bounce and RPG shot.", &rpgtimerToggle));
	saveIf(hud, uiw::settingToggle("RPG angle", "Displays RPG angle on bounce.", &rpgangleToggle));
	saveIf(hud, uiw::settingToggle("5 timing", "", &timing5Toggle));
	saveIf(hud, uiw::settingToggle("Show velocity on bounce", "", &bouncevelocityToggle));
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("Utilities", kMiscCardHeight);
	if (uiw::accentButton("Copy position")) {
		copyPosition(hud);
	}
	uiw::tooltip("Copies origin + view to the clipboard. F3 teleports back on a devmap.");

	ImGui::Dummy(ImVec2(0.f, 10.f));
	readonlyVec3("Copied origin", copiedPositionOrigin);
	readonlyVec3("Copied view", copiedPositionView);

	ImGui::Dummy(ImVec2(0.f, 8.f));
	uiw::settingTextInput("Bind demo name", "Name used by the F load/record bind.", &demoBindName, "demo name");
	if (uiw::accentButton("Bind demo to load key")) {
		bindDemoLoadKey(hud);
	}
	uiw::tooltip("Binds F to load + record this demo name.");
	uiw::endCard();
	uiw::endGrid();
}

void ui_menu::drawCollisionPage(Avengers* hud)
{
	uiw::beginCard("Collision");
	saveIf(hud, uiw::settingToggle("Draw collision", "Visualize world clips. Also processes brushes for jump-target brush mode.", &drawCollision));
	saveIf(hud, uiw::settingToggle("Only clips", "", &drawCollisionOnlyClips));
	saveIf(hud, uiw::settingToggle("Don't draw skies", "", &drawCollisionNoSky));
	saveIf(hud, uiw::settingSlider("Distance", "0 = infinite.", &drawCollisionDistance, 0.f, 15000.f, "%.0f"));
	uiw::endCard();
}

void ui_menu::drawMarkers(Avengers* hud)
{
	ui_position_marker* markers = hud->instUiPositionMarker.get();

	uiw::beginGrid();
	uiw::beginCard("Markers");
	saveIf(hud, uiw::settingToggle("Render markers", "", &renderMarkers));
	saveIf(hud, uiw::settingToggle("Allow binds", "Numpad 1: set marker\nNumpad 2: toggle positioning helper\nNumpad 3: toggle marker rendering", &useMarkerBinds));
	saveIf(hud, uiw::settingToggle("Use legacy markers", "Render a red circle instead of a vertical line.", &useLegacyMarkers));
	saveIf(hud, uiw::settingSlider("Marker render distance", "", &markerRenderDistance, 0.f, 50000.f, "%.0f"));
	saveIf(hud, uiw::settingSlider("Widget render distance", "", &widgetRenderDistance, 0.f, 500.f, "%.0f"));
	saveIf(hud, uiw::settingColor("New marker color", "", &markers->selectedColor));

	const bool connected = hud->instGame->isConnected();
	ImGui::BeginDisabled(!connected);
	if (uiw::accentButton("Add marker")) {
		markers->addMarker();
	}
	ImGui::EndDisabled();
	uiw::endCard();

	uiw::beginCard("Positioning helper");
	saveIf(hud, uiw::settingToggle("Positioning helper", "Displays a positioning helper for nearby markers.", &positioningHelper));
	saveIf(hud, uiw::settingToggle("Only on ground", "Only displays the positioning helper while on the ground.", &positioningHelperOnlyonground));
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("Placed markers");
	int closestIndex = -1;
	if (connected && !markers->markers.empty()) {
		const vec3<float> playerPos = hud->instGame->getOrigin();
		float closestDistance = 0.f;
		for (int i = 0; i < static_cast<int>(markers->markers.size()); ++i) {
			const float dist = markers->markers[i].position.dist(playerPos);
			if (closestIndex == -1 || dist < closestDistance) {
				closestDistance = dist;
				closestIndex = i;
			}
		}
	}

	if (markers->markers.empty()) {
		ImGui::TextDisabled("No markers on this map.");
	}

	const float frameHeight = ImGui::GetFrameHeight();
	const float deleteWidth = ImGui::CalcTextSize("Delete").x + ImGui::GetStyle().FramePadding.x * 2.f;
	const float teleportWidth = ImGui::CalcTextSize("Teleport").x + ImGui::GetStyle().FramePadding.x * 2.f;
	if (!markers->markers.empty() && ImGui::BeginTable(
		"##placedMarkers", 4, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) {
		ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, frameHeight);
		ImGui::TableSetupColumn("Coordinates", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed, deleteWidth);
		ImGui::TableSetupColumn("Teleport", ImGuiTableColumnFlags_WidthFixed, teleportWidth);

		for (int i = 0; i < static_cast<int>(markers->markers.size()); ++i) {
			Marker& marker = markers->markers[i];
			ImGui::PushID(i);
			ImGui::TableNextRow(0, frameHeight);

			ImGui::TableSetColumnIndex(0);
			if (ImGui::ColorButton(
				"##MarkerColor", marker.color, ImGuiColorEditFlags_NoTooltip, ImVec2(frameHeight, frameHeight))) {
				ImGui::OpenPopup("MarkerColorPickerPopup");
			}
			if (ImGui::BeginPopup("MarkerColorPickerPopup")) {
				if (ImGui::ColorPicker4("Marker Color Picker", &marker.color.x)) {
					hud->saveMarkers();
				}
				ImGui::EndPopup();
			}

			const std::string coordinates = std::format("{:.4f}  {:.4f}  {:.4f}",
				marker.position[0], marker.position[1], marker.position[2]);

			ImGui::TableSetColumnIndex(1);
			ImGui::AlignTextToFramePadding();
			const std::string visibleCoordinates =
				ellipsizeText(coordinates, ImGui::GetContentRegionAvail().x);
			ImVec4 coordinateColor = uiTheme().text;
			if (i == closestIndex) {
				coordinateColor = uiTheme().accent;
			}
			ImGui::TextColored(coordinateColor, "%s", visibleCoordinates.c_str());
			if (visibleCoordinates != coordinates) {
				uiw::tooltip(coordinates);
			}

			ImGui::TableSetColumnIndex(2);
			ImGui::PushStyleColor(ImGuiCol_Button, uiTheme().danger);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, uiTheme().danger);
			const bool deleteMarker = ImGui::Button("Delete", ImVec2(deleteWidth, 0.f));
			ImGui::PopStyleColor(2);

			ImGui::TableSetColumnIndex(3);
			if (hud->instGame->isDevmap() &&
				ImGui::Button("Teleport", ImVec2(teleportWidth, 0.f))) {
				hud->instGame->setPosition(marker.position);
				hud->instGame->setView(marker.angles);
			}

			ImGui::PopID();
			if (deleteMarker) {
				markers->markers.erase(markers->markers.begin() + i);
				hud->saveMarkers();
				--i;
			}
		}
		ImGui::EndTable();
	}
	uiw::endCard();
	uiw::endGrid();
}

void ui_menu::drawDemoPlayer(Avengers* hud)
{
	ui_demoplayer* demo = hud->instUiDemoplayer.get();
	demo->timescale = static_cast<float>(atof(demo->timescaleInput.c_str()));
	demo->demoNum = atoi(demo->demoCountInput.c_str());
	const int playFrom = atoi(demo->playFromInput.c_str());
	if (demo->playDemosFrom != playFrom) {
		demo->playDemosIndex = playFrom;
	}
	demo->playDemosFrom = playFrom;

	constexpr float kDemoCardHeight = 460.f;
	uiw::beginGrid();
	uiw::beginCard("Demo", kDemoCardHeight);

	if (uiw::accentButton("Play demos")) {
		hud->wantInput = false;
		demo->playingDemos = true;
	}

	ImGui::Dummy(ImVec2(0.f, 10.f));
	uiw::settingTextInput("Timescale", "", &demo->timescaleInput, "1.0");
	uiw::settingTextInput("Demo count", "", &demo->demoCountInput, "0");
	uiw::settingTextInput("Execute every demo", "", &demo->extraCommandInput);
	uiw::settingTextInput("Play demos from", "", &demo->playFromInput, "0");
	uiw::endCard();

	uiw::nextGridColumn();
	uiw::beginCard("FPS image", kDemoCardHeight);
	uiw::settingToggle("F9 on all demos played", "Simulate F9 when each demo starts and ends (Fraps).", &demo->simF9);
	saveIf(hud, uiw::settingToggle("OBS record on all demos",
		"Starts and stops an OBS recording for each playlist demo.\n\n"
		"In OBS: Tools → WebSocket Server Settings → Enable.\n"
		"Then set Host, Port, and Password below to match OBS.\n\n"
		"This talks to OBS over WebSocket. It does not press F9.",
		&demo->obs.enabled));
	uiw::settingToggle("Show FPS image", "", &demo->showFpsImage);
	uiw::settingToggle("WTMOD", "", &demo->wtmod);
	uiw::settingToggle("3XP", "", &demo->threexp);
	if (demo->wtmod && demo->threexp) {
		demo->threexp = false;
	}
	uiw::settingSlider("FPS image scale", "", &demo->imageScale, 0.01f, 10.f);
	uiw::endCard();
	uiw::endGrid();

	if (demo->obs.enabled) {
		uiw::beginCard("OBS WebSocket");
		saveIf(hud, uiw::settingTextInput("Host", "Usually 127.0.0.1 on the same PC.", &demo->obs.host, "127.0.0.1"));
		saveIf(hud, uiw::settingTextInput("Port", "OBS default is 4455.", &demo->obs.port, "4455"));
		saveIf(hud, uiw::settingTextInput("Password", "From OBS WebSocket Server Settings.", &demo->obs.password));
		uiw::endCard();
	}
}

void ui_menu::menu(Avengers* hud)
{
	ImFont* font = getMenuFont();
	if (font) {
		ImGui::PushFont(font);
	}

	uiw::setTooltipsEnabled(showMenuTooltips);

	ImGui::SetNextWindowSize(ImVec2(980.f, 640.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(840.f, 540.f), ImVec2(1400.f, 900.f));
	ImGui::Begin("Avengers Helper", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	if (font && font->FontSize > 0.f) {
		ImGui::SetWindowFontScale(menuFontSize / font->FontSize);
	}

	if (shouldFocusNextFrame) {
		ImGui::SetWindowFocus();
		shouldFocusNextFrame = false;
	}

	ImGui::SetCursorPos(ImVec2(0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, uiTheme().bgAlt);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 18.f));
	ImGui::BeginChild("##AvengersHelperTabSelector", ImVec2(kSidebarW, 0.f), false,
		ImGuiWindowFlags_AlwaysUseWindowPadding);

	for (size_t i = 0; i < kMenuTabs.size(); ++i) {
		const MenuTab tab = static_cast<MenuTab>(i);
		if (uiw::navButton(kMenuTabs[i].title, activeTab == tab)) {
			activeTab = tab;
		}
		ImGui::Dummy(ImVec2(0.f, 6.f));
	}

	drawSidebarDragArea();

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(ImVec2(kSidebarW, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.f, 24.f));
	ImGui::BeginChild("##AvengersHelperTabContent", ImVec2(0.f, 0.f), false,
		ImGuiWindowFlags_AlwaysUseWindowPadding);

	const MenuTabInfo& activeTabInfo = kMenuTabs[static_cast<size_t>(activeTab)];
	ImGui::Text("[%s]", activeTabInfo.title.c_str());
	ImGui::TextDisabled("%s", activeTabInfo.subtitle.c_str());
	ImGui::Dummy(ImVec2(0.f, 12.f));
	(this->*activeTabInfo.draw)(hud);

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGui::SetWindowFontScale(1.f);
	ImGui::End();

	if (font) {
		ImGui::PopFont();
	}
}

void ui_menu::centerSpeedometer(Avengers* hud)
{
	ImGui::SetWindowFontScale(veloScale);
	ImFont* speedometerFont = getSpeedometerFont();
	if (speedometerFont) {
		ImGui::PushFont(speedometerFont);
	}

	if (keepVeloCentered) {
		veloPos.x = hud->instGame->getScreenRes().x / 2.f - ImGui::CalcTextSize("0").x / 2.f;
	}
	veloPos.y = hud->instGame->getScreenRes().y / 2.f;

	ImGui::SetWindowFontScale(1.f);
	if (speedometerFont) {
		ImGui::PopFont();
	}
	hud->saveConfiguration();
}

void ui_menu::copyPosition(Avengers* hud)
{
	vec3<float> position = hud->instGame->getOrigin();
	const vec3<float> view = hud->instGame->getView();

	copiedPositionOrigin = position;
	copiedPositionView = view;
	hud->saveConfiguration();

	position.z += 60.f;
	std::ostringstream text;
	text << std::fixed << std::setprecision(6)
		<< position.x << ' ' << position.y << ' ' << position.z << ' ' << view.y << ' ' << view.x;
	ImGui::SetClipboardText(text.str().c_str());
}

void ui_menu::bindDemoLoadKey(Avengers* hud)
{
	std::ostringstream command;
	command << "bind f \"openscriptmenu cj load;stoprecord;record";
	if (!demoBindName.empty()) {
		command << ' ' << demoBindName;
	}
	command << '"';
	hud->instGame->sendCommandToConsole(command.str().c_str());
}

void ui_menu::render()
{
	Avengers* hud = Avengers::getInstance();
	
	if(hud->wantInput)
	{
		menu(hud);
	}

	if (showPosition) {
		hud->instUiPosition->render();
		hud->instUiView->render();
	}

	/* WIP
	if (drawfpsToggle && hud->instGame->isConnected()) {
		if ((drawfpsSpectateonly && hud->instGame->isSpectating()) || !drawfpsSpectateonly) {
			ImGui::SetNextWindowSize(ImVec2(500, 500));
			ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
			ImGui::SetWindowFontScale(fpsScale);
			std::string fps = std::to_string(hud->instGame->getFps(true));

			ImGui::PushStyleColor(ImGuiCol_Text, fpsColor);
			ImGui::Text(fps.c_str());
			ImGui::PopStyleColor();

			ImGui::SetWindowFontScale(1.f);
			ImGui::End();
		}
	}*/

	//Render speedometer
	if ((veloMeter) && hud->instGame->isConnected())
	{
		hud->instUiVelocity->render(hud, lockVeloPos, veloPos, veloScale, color);
	}

	if ((drawJumpoffSpeed || jumpoffspeedDisplayBottom) && hud->instGame->isConnected()) {
		ImVec4 color(0.9f, 0.3f, 0.75f, 1.f);
		hud->instUiVelocity->renderJumpoffSpeed(hud, veloPos, veloScale, color);
	}

	if (drawcenterline && hud->instGame->isConnected()) {
		hud->instUiAnglehelper->renderCenterLine(hud, centerlineColor);
	}

	//Render anglehelper
	if (anglehelperToggle && hud->instGame->isConnected())
	{
		hud->instUiAnglehelper->render(hud, anglehelperColor);
	}

	if (fpswheelToggle && hud->instGame->isConnected())
	{
		hud->instUiFpswheel->render(hud);
	}

	//Jump Target
	if(jumpTarget && hud->instGame->isConnected())
	{
		hud->instUiJumpTarget->render();
	}

	//Strafe downtime
	if (strafedowntimeToggle && hud->instGame->isConnected()) {
		hud->instUiStrafedowntime->render();
	}

	//bounce info
	if (rpgtimerToggle && hud->instGame->isConnected()) {
		hud->instUiBounceinfo->renderRpgTimer();
	}

	if (bouncevelocityToggle && hud->instGame->isConnected()) {
		hud->instUiBounceinfo->renderBounceVelocity();
	}

	if (rpgangleToggle && hud->instGame->isConnected()) {
		hud->instUiBounceinfo->renderRpgAngle();
	}

	if (timing5Toggle && hud->instGame->isConnected()) {
		hud->instUiBounceinfo->render5Timing();
	}

	hud->instUiPositionMarker->render();

	hud->collision->init();
}

void ui_menu::registerConfigs(Avengers* hud)
{
	hud->registerConfig("Speedometer", &veloMeter);
	hud->registerConfig("SpeedometerFont", &selectedSpeedometerFont);
	hud->registerConfig("MenuFont", &selectedMenuFont);
	hud->registerConfig("MenuFontSize", &menuFontSize);
	hud->registerConfig("Position", &veloPos);
	hud->registerConfig("Color_anglehelper", &anglehelperColor);
	hud->registerConfig("Color_90_lines", &linesColor);
	hud->registerConfig("Color", &color);
	hud->registerConfig("Scale", &veloScale);
	hud->registerConfig("PosHud", &showPosition);
	hud->registerConfig("LastCopiedPositionOrigin", &copiedPositionOrigin);
	hud->registerConfig("LastCopiedPositionView", &copiedPositionView);
	hud->registerConfig("Anglehelper", &anglehelperToggle);
	hud->registerConfig("90_Lines", &linesToggle);
	hud->registerConfig("FPSWheel", &fpswheelToggle);
	hud->registerConfig("FPSWheelOffsetY", &fpswheelOffsetY);
	hud->registerConfig("FPSWheelOffsetX", &fpswheelOffsetX);
	hud->registerConfig("FPSWheelSize", &fpswheelSize);
	hud->registerConfig("Anglehelper_pixel_scale", &ahPixelScale);
	hud->registerConfig("Wheel_anglehelper_pixel_scale", &wheelAhPixelScale);
	hud->registerConfig("Wheel_pixel_scale", &wheelPixelScale);
	hud->registerConfig("JumpoffSpeed", &drawJumpoffSpeed);
	hud->registerConfig("JumpoffSpeed_bottom", &jumpoffspeedDisplayBottom);
	hud->registerConfig("Strafedowntime", &strafedowntimeToggle);
	hud->registerConfig("rpgtimer", &rpgtimerToggle);
	hud->registerConfig("bouncevelocity", &bouncevelocityToggle);
	hud->registerConfig("drawfps", &drawfpsToggle);
	hud->registerConfig("color_fps", &fpsColor);
	hud->registerConfig("scale_fps", &fpsScale);
	hud->registerConfig("drawfps_spectateonly", &drawfpsSpectateonly);
	hud->registerConfig("rpgangle", &rpgangleToggle);
	hud->registerConfig("5timing", &timing5Toggle);
	hud->registerConfig("centerline_toggle", &drawcenterline);
	hud->registerConfig("centerline_width", &centerlineWidth);
	hud->registerConfig("color_centerline", &centerlineColor);
	hud->registerConfig("centerline_toggle_fpswheel", &drawfpswheelcenterline);
	hud->registerConfig("color_fpswheelcenterline", &fpswheelcenterlineColor);
	hud->registerConfig("velo_acceleration_toggle", &veloShowAcceleration);
	hud->registerConfig("velo_deceleration_toggle", &veloShowDeceleration);
	hud->registerConfig("color_acceleration", &accelerationColor);
	hud->registerConfig("color_deceleration", &decelerationColor);
	hud->registerConfig("keep_velo_centered_toggle", &keepVeloCentered);
	hud->registerConfig("ah_style", &currentAhStyle);
	hud->registerConfig("clamp_to_next_zone", &clampToNextZone);
	hud->registerConfig("anglehelper_y_offset", &anglehelperYOffset);
	hud->registerConfig("draw_collision", &drawCollision);
	hud->registerConfig("draw_collision_only_clips", &drawCollisionOnlyClips);
	hud->registerConfig("draw_collision_distance", &drawCollisionDistance);
	hud->registerConfig("draw_collision_no_sky", &drawCollisionNoSky);
	hud->registerConfig("brush_mode", &brushMode);
	hud->registerConfig("draw_selected_brushes", &drawSelectedBrushes);
	hud->registerConfig("jump_target", &jumpTarget);
	hud->registerConfig("anglehelper_height", &anglehelperHeight);
	hud->registerConfig("anglehelper_width", &anglehelperWidth);
	hud->registerConfig("use_static_positioning", &useStaticPositioning);
	hud->registerConfig("marker_color", &hud->instUiPositionMarker->selectedColor);
	hud->registerConfig("render_markers", &renderMarkers);
	hud->registerConfig("use_binds", &useMarkerBinds);
	hud->registerConfig("marker_render_distance", &markerRenderDistance);
	hud->registerConfig("widget_render_distance", &widgetRenderDistance);
	hud->registerConfig("positioning_helper", &positioningHelper);
	hud->registerConfig("positioning_helper_onlyonground", &positioningHelperOnlyonground);
	hud->registerConfig("jump_target_select_closest", &jumpTargetSelectClosest);
	hud->registerConfig("allow_impure_map_iwds", &allowImpureMapIwds);
	hud->registerConfig("show_menu_tooltips", &showMenuTooltips);
	hud->registerConfig("use_legacy_markers", &useLegacyMarkers);
	hud->registerConfig("velo_acceleration_threshold", &veloAccelerationThreshold);
	hud->registerConfig("velo_deceleration_threshold", &veloDecelerationThreshold);
	hud->registerConfig("velo_keep_accel_for", &veloKeepAccelFor);
	hud->registerConfig("velo_keep_decel_for", &veloKeepDecelFor);
	hud->registerConfig("enable_acceleration_on_ground", &enableAccelerationOnGround);
	hud->registerConfig("enable_deceleration_on_ground", &enableDecelerationOnGround);
	hud->registerConfig("obs_websocket", &hud->instUiDemoplayer->obs.enabled);
	hud->registerConfig("obs_websocket_host", &hud->instUiDemoplayer->obs.host);
	hud->registerConfig("obs_websocket_port", &hud->instUiDemoplayer->obs.port);
	hud->registerConfig("obs_websocket_password", &hud->instUiDemoplayer->obs.password);
}

ImFont* ui_menu::lookupFont(const std::string& name) const
{
	const auto font = loadedFonts.find(name);
	if (font == loadedFonts.end()) {
		return nullptr;
	}
	return font->second;
}

ImFont* ui_menu::getMenuFont() const
{
	if (ImFont* font = lookupFont(selectedMenuFont)) {
		return font;
	}
	return lookupFont("Trebuchet");
}

ImFont* ui_menu::getSpeedometerFont() const
{
	if (ImFont* font = lookupFont(selectedSpeedometerFont)) {
		return font;
	}
	return lookupFont("Bahnschrift");
}

ui_menu::ui_menu(Avengers* hud)
{
	hud->instRender->addCallback([this]() { this->render(); });
}
