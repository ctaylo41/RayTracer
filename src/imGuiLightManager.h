#ifndef IMGUI_LIGHT_MANAGER_H
#define IMGUI_LIGHT_MANAGER_H

#include "light.h"
#include "lightManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>
#include "camera.h"
class ImGuiLightManager {
public:
    ImGuiLightManager(LightManager& lightManager, Camera& camera) : lightManager(lightManager), camera(camera) {}

    void render();

    void renderLightList();
    void renderLightEditor();
    void renderLightProperties();
    void renderPresetButtons();
    void renderAddLightPanel();

    void setVisible(bool visible) { showWindow = visible; }
    bool isVisible() const { return showWindow; }
    

private:
    LightManager& lightManager;
    bool showWindow = true;
    int selectedLightIndex = -1;

    bool showAddLightPanel = false;
    int newLightType = 0;
    float newLightPosition[3] = { 0.0f, 5.0f, 0.0f };
    float newLightDirection[3] = { 0.0f, -1.0f, 0.0f };
    float newLightColor[3] = { 1.0f, 1.0f, 1.0f };
    float newLightIntensity = 1.0f;
    float newSpotInner = 12.5f;
    float newSpotOuter = 17.5f;

    const char* getLightTypeName(LightType type) const;
    ImVec4 getLightTypeColor(LightType type) const;
    void renderLightControls(Light& light, int index);
    void renderDirectionalLightControls(Light& light);
    void renderSpotLightControls(Light& light);
    void renderPointLightControls(Light& light);
    void addNewLight();
    Camera& camera; // Add a reference to the camera

};

#endif