#include "pch.h"
#include "ui_position_marker.h"

ImU32 ui_position_marker::imVec4ToImCol32(const ImVec4& color)
{
    return IM_COL32(
        static_cast<int>(color.x * 255),
        static_cast<int>(color.y * 255),
        static_cast<int>(color.z * 255),
        static_cast<int>(color.w * 255));
}

ImVec4 ui_position_marker::invertColor(const ImVec4& color)
{
    return ImVec4(1.f - color.x, 1.f - color.y, 1.f - color.z, color.w);
}

ui_position_marker::ui_position_marker(Avengers* avengers) :
    avengers(avengers)
{
    avengers->instInput->addCallback(VK_NUMPAD2, [this](UINT keyState) { return this->bindToggleWidget(keyState); });
    avengers->instInput->addCallback(VK_NUMPAD1, [this](UINT keyState) { return this->bindSetMarker(keyState); });
    avengers->instInput->addCallback(VK_NUMPAD3, [this](UINT keyState) { return this->bindToggleRenderMarkers(keyState); });
}

ui_position_marker::~ui_position_marker()
{
}

void ui_position_marker::render()
{
    bool isConnected = avengers->instGame->isConnected();

    if (isConnected) {
        std::string currentMap = avengers->instGame->getMapName();
        if (initializedForMap != currentMap) {
            avengers->loadMarkers();
            initializedForMap = currentMap;
        }
    }
    else {
        return;
    }

    if (!avengers->instUiMenu->renderMarkers) {
        return;
    }

    constexpr float LINE_HEIGHT = 40.f;
    constexpr float LINE_WIDTH = 20.f;
    constexpr float CIRCLE_RADIUS = 10.f;
    constexpr float ANGLE_WIDTH = 20.f;
    constexpr float FADEOUT_DIST = 150.f;

    bool centerDrawn = false;
    for (std::size_t i = 0; i < markers.size(); i++) {
        vec3<float> playerPos = avengers->instGame->getOrigin();
        float lineWidth = LINE_WIDTH;

        Marker& marker = markers[i];
        if (marker.position.dist(playerPos) >= avengers->instUiMenu->markerRenderDistance) {
            continue;
        }

        float markerDist = marker.position.dist(playerPos);
        if (markerDist >= FADEOUT_DIST) {
            lineWidth = std::fmaxf(LINE_WIDTH * (FADEOUT_DIST / (markerDist * 2.f)), 1.f);
        }

        ImVec4 color = marker.color;
        vec3<float> pos1 = marker.position;
        vec3<float> pos2 = pos1;
        pos2.z += LINE_HEIGHT;

        vec3<float> anglingHelper1(pos1.x, pos1.y, pos1.z + LINE_HEIGHT);
        vec3<float> anglingHelper2(pos1.x + ANGLE_WIDTH, pos1.y, pos1.z + LINE_HEIGHT);
        float angleToRotate = marker.angles.y;
        float angleToRotate180 = mm::normalise(marker.angles.y + 180.f, 0.f, 360.f);
        vec2<float> rotated = mm::rotatePoint(vec2<float>(anglingHelper2.x, anglingHelper2.y), vec2<float>(anglingHelper1.x, anglingHelper1.y), angleToRotate);
        anglingHelper2.x = rotated.x;
        anglingHelper2.y = rotated.y;

        ImDrawList* backgroundDrawList = ImGui::GetBackgroundDrawList();

        if (!avengers->instUiMenu->useLegacyMarkers) {
            ImVec2 screen1;
            ImVec2 screen2;
            ImVec2 screenAngle1;
            ImVec2 screenAngle2;

            bool v1 = avengers->instGame->worldToScreen(pos1, &screen1.x, &screen1.y);
            bool v2 = avengers->instGame->worldToScreen(pos2, &screen2.x, &screen2.y);

            bool sa1 = avengers->instGame->worldToScreen(anglingHelper1, &screenAngle1.x, &screenAngle1.y);
            bool sa2 = avengers->instGame->worldToScreen(anglingHelper2, &screenAngle2.x, &screenAngle2.y);

            if (v1 && v2) {
                backgroundDrawList->AddLine(screen1, screen2, imVec4ToImCol32(color), lineWidth);
                backgroundDrawList->AddCircleFilled(screen1, CIRCLE_RADIUS, imVec4ToImCol32(invertColor(color)), 36.f);
            }

            if (sa1 && sa2) {
                backgroundDrawList->AddLine(screenAngle1, screenAngle2, imVec4ToImCol32(invertColor(color)), lineWidth);
            }
        }
        else {
            ImVec2 screen;
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            avengers->instGame->worldToScreen(marker.position, &screen.x, &screen.y);

            ImU32 outlineColor = imVec4ToImCol32(color);
            int numSegments = 8;

            float distance = avengers->gameState->origin.dist(marker.position);

            const float minRadius = 1.0f;
            const float maxRadius = CIRCLE_RADIUS * 5;

            //make the circle get smaller and disappear if the player is over 512 units from the centre
            unsigned int newRadius = minRadius + (maxRadius - minRadius) * (minRadius - distance / 512.0f);

            drawList->AddCircle(screen, newRadius, outlineColor, numSegments, 2);
        }

        constexpr float HELPER_CIRCLE_RADIUS = 30.f;
        constexpr float WINDOW_SIZE = 200.f;
        constexpr float WINDOW_OFFSET = 200.f;
        float widgetRenderDist = avengers->instUiMenu->widgetRenderDistance;

        if (((avengers->instUiMenu->positioningHelperOnlyonground && avengers->gameState->onGround) || !avengers->instUiMenu->positioningHelperOnlyonground)
            && avengers->instUiMenu->positioningHelper && marker.position.dist(playerPos) <= widgetRenderDist) {
            vec3<float> dist = playerPos - marker.position;

            ImGui::SetNextWindowSize(ImVec2(WINDOW_SIZE, WINDOW_SIZE));
            ImGui::SetNextWindowPos(ImVec2(avengers->instGame->getScreenRes().x / 2.f - WINDOW_SIZE / 2.f, avengers->instGame->getScreenRes().y / 2.f - WINDOW_SIZE / 2.f + WINDOW_OFFSET));
            
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
            ImGui::Begin("Positioning Helper", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

            ImVec2 centerPos = ImVec2(ImGui::GetWindowPos().x + (ImGui::GetWindowSize().x / 2.f), ImGui::GetWindowPos().y + (ImGui::GetWindowSize().y / 2.f));
            ImVec2 markerPos = ImVec2(centerPos.x - dist.x, centerPos.y + dist.y);

            ImGui::GetWindowDrawList()->AddCircleFilled(markerPos, HELPER_CIRCLE_RADIUS, imVec4ToImCol32(marker.color));
            vec2<float> rotated = mm::rotatePoint(vec2<float>(markerPos.x - HELPER_CIRCLE_RADIUS - 20.f, markerPos.y), vec2<float>(markerPos.x, markerPos.y), mm::normalise(360.f - angleToRotate180, 0.f, 360.f));
            ImGui::GetWindowDrawList()->AddLine(markerPos, ImVec2(rotated.x, rotated.y), imVec4ToImCol32(invertColor(marker.color)), 1.f);

            if (!centerDrawn) {
                ImGui::GetWindowDrawList()->AddCircleFilled(centerPos, HELPER_CIRCLE_RADIUS, ImColor(0.5f, 0.5f, 0.5f, 0.8f));
                vec2<float> rotatedCenter = mm::rotatePoint(vec2<float>(centerPos.x + HELPER_CIRCLE_RADIUS + 20.f, centerPos.y), vec2<float>(centerPos.x, centerPos.y), mm::normalise(-1.f * avengers->instGame->getView().y, 0.f, 360.f));
                
                ImGui::GetWindowDrawList()->AddLine(centerPos, ImVec2(rotatedCenter.x, rotatedCenter.y), ImColor(1.f, 1.f, 1.f, 1.f), 1.f);
                centerDrawn = true;
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }
}

void ui_position_marker::addMarker()
{
    Marker marker(avengers->gameState->origin, avengers->instGame->getView(), selectedColor);
    markers.push_back(marker);
    avengers->saveMarkers();
}

bool ui_position_marker::bindSetMarker(UINT keyState)
{
    if (keyState == WM_KEYUP) {
        if (avengers->instUiMenu->useMarkerBinds) {
            avengers->instGame->addObituary("Marker set");
            addMarker();
        }
    }
    return false;
}

bool ui_position_marker::bindToggleWidget(UINT keyState)
{
    if (keyState == WM_KEYUP) {
        if (avengers->instUiMenu->useMarkerBinds) {
            avengers->instGame->addObituary("Positioning widget toggled");
            avengers->instUiMenu->positioningHelper = !avengers->instUiMenu->positioningHelper;
        }
    }
    return false;
}

bool ui_position_marker::bindToggleRenderMarkers(UINT keyState)
{
    if (keyState == WM_KEYUP) {
        if (avengers->instUiMenu->useMarkerBinds) {
            avengers->instGame->addObituary("Marker rendering toggled");
            avengers->instUiMenu->renderMarkers = !avengers->instUiMenu->renderMarkers;
        }
    }
    return false;
}

void ui_position_marker::menu()
{
    int closestMarkerIndex = -1;
    if (avengers->instGame->isConnected() && !markers.empty()) {
        vec3<float> playerPos = avengers->instGame->getOrigin();
        float closestDistance = 0.f;

        for (int i = 0; i < static_cast<int>(markers.size()); ++i) {
            float markerDistance = markers[i].position.dist(playerPos);
            if (closestMarkerIndex == -1 || markerDistance < closestDistance) {
                closestDistance = markerDistance;
                closestMarkerIndex = i;
            }
        }
    }

    bool isConnected = avengers->instGame->isConnected();

    if (!isConnected) {
        ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
    }
    if (ImGui::Button("Add Marker")) {
        addMarker();
    }

    if (!isConnected) {
        ImGui::PopItemFlag();
    }

    ImGui::SameLine();
    ImGui::ColorButton("##Marker Color", selectedColor);

    if (ImGui::IsItemClicked()) {
        ImGui::OpenPopup("MarkerColorPickerPopup");
    }

    if (ImGui::BeginPopup("MarkerColorPickerPopup")) {
        ImGui::ColorPicker4("Marker Color Picker", &selectedColor.x);
        ImGui::EndPopup();
        avengers->saveConfiguration();
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Render Markers", &avengers->instUiMenu->renderMarkers)) {
        avengers->saveConfiguration();
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Allow Binds", &avengers->instUiMenu->useMarkerBinds)) {
        avengers->saveConfiguration();
    }
    ImGui::SetItemTooltip("Numpad 1: Set a marker\nNumpad 2: Toggle the positioning helper\nNumpad 3: Toggle marker rendering");
    ImGui::PushItemWidth(430.f);
    if (ImGui::SliderFloat("Marker Render Distance", &avengers->instUiMenu->markerRenderDistance, 0.f, 50000.f)) {
        avengers->saveConfiguration();
    }
    if (ImGui::Checkbox("Positioning Helper", &avengers->instUiMenu->positioningHelper)) {
        avengers->saveConfiguration();
    }
    ImGui::SetItemTooltip("Displays a positioning helper for nearby markers");
    ImGui::SameLine();
    if (ImGui::Checkbox("Only on ground", &avengers->instUiMenu->positioningHelperOnlyonground)) {
        avengers->saveConfiguration();
    }
    ImGui::SetItemTooltip("Only displays the positioning helper while on the ground");
    ImGui::SameLine();
    if (ImGui::Checkbox("Use legacy markers", &avengers->instUiMenu->useLegacyMarkers)) {
        avengers->saveConfiguration();
    }
    ImGui::SetItemTooltip("Renders a red circle instead of a vertical line");
    if (ImGui::SliderFloat("Widget Render Distance", &avengers->instUiMenu->widgetRenderDistance, 0.f, 500.f)) {
        avengers->saveConfiguration();
    }
    ImGui::PopItemWidth();

    ImGui::Separator();
    for (int i = 0; i < static_cast<int>(markers.size()); ++i) {
        Marker& m = markers[i];

        ImGui::PushID(i);
        bool isClosestMarker = i == closestMarkerIndex;
        if (isClosestMarker) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 1.f, 1.f));
        }

        ImGui::ColorButton("##MarkerColor", m.color);
        std::string popupName = std::string("MarkerColorPickerPopup_") + std::to_string(i);
        if (ImGui::IsItemClicked()) {
            ImGui::OpenPopup(popupName.c_str());
        }

        ImGui::SameLine();
        ImGui::Text("%.4f %.4f %.4f", m.position[0], m.position[1], m.position[2]);
        ImGui::SameLine();

        if (isClosestMarker) {
            ImGui::PopStyleColor();
        }

        if (ImGui::Button("Delete")) {
            markers.erase(markers.begin() + i);
            avengers->saveMarkers();
            if (isClosestMarker) {
                closestMarkerIndex = -1;
            }
            ImGui::PopID();
            --i;
            continue;
        }

        if (avengers->instGame->isDevmap()) {
            ImGui::SameLine();
            if (ImGui::Button("Teleport")) {
                avengers->instGame->setPosition(m.position);
                avengers->instGame->setView(m.angles);
            }
        }

        if (ImGui::BeginPopup(popupName.c_str())) {
            ImGui::ColorPicker4("Marker Color Picker", &m.color.x);
            ImGui::EndPopup();
            avengers->saveMarkers();
        }

        ImGui::PopID();
    }

}
