#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "vectors.h"
#include "config_manager.h"

enum class MenuTab
{
	General,
	Velocity,
	JumpTarget,
	AngleHelper,
	FpsWheel,
	Misc,
	Collision,
	Markers,
	DemoPlayer
};

class ui_menu
{
public:
	ui_menu(class Avengers* hud);
	~ui_menu() = default;
	void menu(Avengers* hud);
	void render();

	MenuTab activeTab = MenuTab::General;
	bool showMenuTooltips = true;
	bool showPosition = false;
	bool veloMeter = false;
	bool keepVeloCentered = false;
	bool useStaticPositioning = false;
	bool veloShowAcceleration = false;
	bool veloShowDeceleration = false;
	float veloAccelerationThreshold = 10.f;
	float veloDecelerationThreshold = 10.f;
	float veloKeepAccelFor = 10.f;
	float veloKeepDecelFor = 50.f;
	bool enableAccelerationOnGround = false;
	bool enableDecelerationOnGround = false;
	bool drawJumpoffSpeed = false;
	bool jumpoffspeedDisplayBottom = false;
	ImVec4 color = { 0.0f, 0.0f, 1.0f, 1.0f };
	ImVec4 accelerationColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	ImVec4 decelerationColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	ImVec4 linesColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	ImVec4 anglehelperColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	float veloScale = 1.5;
	bool lockVeloPos = true;
	bool anglehelperToggle = false;
	float anglehelperYOffset = 0.0f;
	float anglehelperWidth = 1.f;
	float anglehelperHeight = 1.f;
	bool clampToNextZone = false;
	bool drawcenterline = false;
	float centerlineWidth = 2.5f;
	ImVec4 centerlineColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool drawfpswheelcenterline = false;
	ImVec4 fpswheelcenterlineColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool fpswheelToggle = false;
	float fpswheelSize = 20.f;
	float fpswheelOffsetY = 0.f;
	float fpswheelOffsetX = 1.f;
	float ahPixelScale = 0.4f;
	float wheelAhPixelScale = 0.4f;
	float wheelPixelScale = 0.4f;
	bool linesToggle = false;
	bool strafedowntimeToggle = false;
	bool rpgtimerToggle = false;
	bool rpgangleToggle = false;
	bool timing5Toggle = false;
	bool bouncevelocityToggle = false;
	bool drawCollision = false;
	bool drawCollisionOnlyClips = false;
	bool drawCollisionNoSky = false;
	float drawCollisionDistance = 5000.f;
	bool renderMarkers = false;
	bool positioningHelper = false;
	bool positioningHelperOnlyonground = false;
	float markerRenderDistance = 500.f;
	float widgetRenderDistance = 50.f;
	bool useMarkerBinds = false;
	bool useLegacyMarkers = false;
	bool allowImpureMapIwds = false;

	// Default the position to the center of the screen if there is no position in the config file
	vec2<float> veloPos = vec2<float>(GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2);

	std::string demoBindName;
	vec3<float> copiedPositionView;
	vec3<float> copiedPositionOrigin;
	
	vec2<float> pos1;
	vec2<float> pos2;
	vec2<float> pos3;

	bool jumpTarget = false;
	bool brushMode = false;
	bool drawSelectedBrushes = false;
	bool jumpTargetSelectClosest = false;
	vec3<float> jumpTargetOrigin {};
	bool drawfpsToggle = false;
	bool drawfpsSpectateonly = false;
	float fpsScale = 1.f;
	ImVec4 fpsColor = { 0.0f, 1.0f, 0.0f, 1.0f };

	bool shouldFocusNextFrame = false;
	std::string currentAhStyle = "Style 1";
	std::string selectedSpeedometerFont = "Bahnschrift";
	std::string selectedMenuFont = "Trebuchet";
	float menuFontSize = 16.f;
	std::unordered_map<std::string, ImFont*> loadedFonts;
	ImFont* getMenuFont() const;
	ImFont* getSpeedometerFont() const;
	void registerConfigs(Avengers* hud);
	void centerSpeedometer(Avengers* hud);
	void copyPosition(Avengers* hud);
	void bindDemoLoadKey(Avengers* hud);

private:
	static constexpr float kSidebarW = 196.f;

	struct MenuTabInfo
	{
		std::string title;
		std::string subtitle;
		void (ui_menu::*draw)(Avengers* hud);
	};

	// Indexed by MenuTab.
	static const std::array<MenuTabInfo, 9> kMenuTabs;

	ImFont* lookupFont(const std::string& name) const;
	bool fontCombo(const std::string& label, const std::string& desc, std::string* selected, const std::string& fallback);
	void drawGeneral(Avengers* hud);
	void drawVelocity(Avengers* hud);
	void drawJumpTarget(Avengers* hud);
	void drawAngleHelper(Avengers* hud);
	void drawFpsWheel(Avengers* hud);
	void drawMisc(Avengers* hud);
	void drawCollisionPage(Avengers* hud);
	void drawMarkers(Avengers* hud);
	void drawDemoPlayer(Avengers* hud);
};

