#include "imGuiLightManager.h"

void ImGuiLightManager::render() {
    if(!showWindow) return;
    ImGui::SetNextWindowPos(ImVec2(1200 - 400, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Light Manager", &showWindow)) {
        glm::vec3 camPos = camera.getPosition();
        glm::vec3 camDir = camera.getFront(); // Or getDirection(), depending on your API
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
        ImGui::Text("View Direction: (%.2f, %.2f, %.2f)", camDir.x, camDir.y, camDir.z);
        ImGui::Separator();
        renderLightProperties();
        ImGui::Separator();

        renderPresetButtons();
        ImGui::Separator();

        if(ImGui::Button("Add new Light")) {
            showAddLightPanel = !showAddLightPanel;
        }

        if(ImGui::Button("Add New Light")) {
            showAddLightPanel = !showAddLightPanel;
        }

        if (showAddLightPanel) {
            renderAddLightPanel();
        }

        ImGui::Separator();
        renderLightList();
        ImGui::Separator();
        renderLightEditor();
    }
    ImGui::End();
}

void ImGuiLightManager::renderLightProperties() {
    LightStats totalLights = lightManager.getStats();
    ImGui::Text("Light Stats");
    ImGui::Text("Total Lights: %d | Enabled Lights: %d", totalLights.totalLights, totalLights.enabledLights);
    ImGui::Text("Directional Lights: %d | Point Lights: %d | Spot Lights: %d", totalLights.directionalLights, totalLights.pointLights, totalLights.spotLights);
    ImGui::Text("Average Intensity: %.2f", totalLights.averageIntensity);
}

void ImGuiLightManager::renderPresetButtons() {
    ImGui::Text("Lighting Presets:");
    
    if (ImGui::Button("Studio Lighting")) {
        lightManager.createStudioLighting();
        selectedLightIndex = -1;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Night Scene")) {
        lightManager.createNightLighting();
        selectedLightIndex = -1;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Golden Hour")) {
        lightManager.createGoldenHourLighting();
        selectedLightIndex = -1;
    }
    
    if (ImGui::Button("Clear All Lights")) {
        lightManager.removeAllLights();
        selectedLightIndex = -1;
    }
}

void ImGuiLightManager::renderAddLightPanel() {
    ImGui::BeginChild("AddLightPanel", ImVec2(0, 200), true);


    ImGui::Text("Add New Light");
    
    // Light type selection
    const char* lightTypes[] = { "Directional", "Point", "Spot" };
    ImGui::Combo("Type", &newLightType, lightTypes, 3);
    
    // Common properties
    ImGui::ColorEdit3("Color", newLightColor);
    ImGui::SliderFloat("Intensity", &newLightIntensity, 0.0f, 20.0f);
    
    // Type-specific properties
    if (newLightType == 0) { // Directional
        ImGui::SliderFloat3("Direction", newLightDirection, -1.0f, 1.0f);
    }
    else if (newLightType == 1) { // Point
        ImGui::SliderFloat3("Position", newLightPosition, -50.0f, 50.0f);
    }
    else if (newLightType == 2) { // Spot
        ImGui::SliderFloat3("Position", newLightPosition, -50.0f, 50.0f);
        ImGui::SliderFloat3("Direction", newLightDirection, -1.0f, 1.0f);
        ImGui::SliderFloat("Inner Cutoff", &newSpotInner, 5.0f, 45.0f);
        ImGui::SliderFloat("Outer Cutoff", &newSpotOuter, 10.0f, 60.0f);
        if (newSpotOuter <= newSpotInner) newSpotOuter = newSpotInner + 5.0f;
    }
    
    if (ImGui::Button("Add Light")) {
        addNewLight();
        showAddLightPanel = false;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Cancel")) {
        showAddLightPanel = false;
    }
    
    ImGui::EndChild();
}

void ImGuiLightManager::renderLightList() {
    ImGui::Text("Lights (%zu)", lightManager.getLightCount());
    
    if (ImGui::BeginChild("LightList", ImVec2(0, 150), true)) {
        for (size_t i = 0; i < lightManager.getLightCount(); ++i) {
            const Light& light = lightManager.getLight(i);
            
            // Light enabled checkbox
            bool enabled = light.getProperties().enabled;
            if (ImGui::Checkbox(("##enabled" + std::to_string(i)).c_str(), &enabled)) {
                lightManager.enableLight(i, enabled);
            }
            ImGui::SameLine();
            
            // Light type colored indicator
            ImVec4 typeColor = getLightTypeColor(light.getType());
            ImGui::TextColored(typeColor, "●");
            ImGui::SameLine();
            
            // Light name/info
            std::string lightName = std::string(getLightTypeName(light.getType())) + " " + std::to_string(i);
            bool isSelected = (static_cast<int>(i) == selectedLightIndex);
            
            if (ImGui::Selectable((lightName + "##" + std::to_string(i)).c_str(), isSelected)) {
                selectedLightIndex = static_cast<int>(i);
            }
            
            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete")) {
                    lightManager.removeLight(i);
                    if (selectedLightIndex >= static_cast<int>(i)) {
                        selectedLightIndex--;
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
            ImGui::SameLine();
            ImGui::Text("I:%.1f", light.getProperties().intensity);
        }
    }
    ImGui::EndChild();
}

void ImGuiLightManager::renderLightEditor() {
    if (selectedLightIndex < 0 || selectedLightIndex >= static_cast<int>(lightManager.getLightCount())) {
        ImGui::Text("No light selected");
        return;
    }
    
    Light& light = lightManager.getLight(selectedLightIndex);
    
    ImGui::Text("Editing Light %d (%s)", selectedLightIndex, getLightTypeName(light.getType()));
    
    if (ImGui::BeginChild("LightEditor", ImVec2(0, 0), true)) {
        renderLightControls(light, selectedLightIndex);
    }
    ImGui::EndChild();
}

void ImGuiLightManager::renderLightControls(Light& light, int index) {
    LightProperties& props = light.getProperties();
    
    // Enable/Disable
    ImGui::Checkbox("Enabled", &props.enabled);
    
    // Color
    float color[3] = {props.color.r, props.color.g, props.color.b};
    if (ImGui::ColorEdit3("Color", color)) {
        props.color = glm::vec3(color[0], color[1], color[2]);
    }
    
    // Intensity
    ImGui::SliderFloat("Intensity", &props.intensity, 0.0f, 20.0f);
    
    // Type-specific controls
    switch (light.getType()) {
        case LightType::Directional:
            renderDirectionalLightControls(light);
            break;
        case LightType::Point:
            renderPointLightControls(light);
            break;
        case LightType::Spot:
            renderSpotLightControls(light);
            break;
    }
    
    // Shadow settings (for future use)
    ImGui::Separator();
    ImGui::Text("Shadows (Future Feature)");
    ImGui::Checkbox("Cast Shadows", &props.castsShadows);
}

void ImGuiLightManager::renderDirectionalLightControls(Light& light) {
    LightProperties& props = light.getProperties();
    
    ImGui::Text("Directional Light Settings");
    
    float dir[3] = {props.direction.x, props.direction.y, props.direction.z};
    if (ImGui::SliderFloat3("Direction", dir, -1.0f, 1.0f)) {
        light.setDirection(glm::vec3(dir[0], dir[1], dir[2]));
    }
    
    // Quick direction presets
    if (ImGui::Button("Sun High")) {
        light.setDirection(glm::vec3(0.3f, -0.8f, 0.5f));
    }
    ImGui::SameLine();
    if (ImGui::Button("Sun Low")) {
        light.setDirection(glm::vec3(0.8f, -0.3f, 0.5f));
    }
    ImGui::SameLine();
    if (ImGui::Button("Overhead")) {
        light.setDirection(glm::vec3(0.0f, -1.0f, 0.0f));
    }
}

void ImGuiLightManager::renderPointLightControls(Light& light) {
    LightProperties& props = light.getProperties();
    
    ImGui::Text("Point Light Settings");
    
    float pos[3] = {props.position.x, props.position.y, props.position.z};
    if (ImGui::SliderFloat3("Position", pos, -50.0f, 50.0f)) {
        light.setPosition(glm::vec3(pos[0], pos[1], pos[2]));
    }
    
    ImGui::Text("Attenuation");
    ImGui::SliderFloat("Constant", &props.constant, 0.0f, 2.0f);
    ImGui::SliderFloat("Linear", &props.linear, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic", &props.quadratic, 0.0f, 1.0f);
    
    // Show calculated range
    float range = light.calculateRange();
    ImGui::Text("Effective Range: %.1f units", range);
    
    // Preset attenuation values
    if (ImGui::Button("Close Range")) {
        light.setAttenuation(1.0f, 0.35f, 0.44f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Medium Range")) {
        light.setAttenuation(1.0f, 0.09f, 0.032f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Far Range")) {
        light.setAttenuation(1.0f, 0.027f, 0.0028f);
    }
}

void ImGuiLightManager::renderSpotLightControls(Light& light) {
    LightProperties& props = light.getProperties();
    
    ImGui::Text("Spot Light Settings");
    
    float pos[3] = {props.position.x, props.position.y, props.position.z};
    if (ImGui::SliderFloat3("Position", pos, -50.0f, 50.0f)) {
        light.setPosition(glm::vec3(pos[0], pos[1], pos[2]));
    }
    
    float dir[3] = {props.direction.x, props.direction.y, props.direction.z};
    if (ImGui::SliderFloat3("Direction", dir, -1.0f, 1.0f)) {
        light.setDirection(glm::vec3(dir[0], dir[1], dir[2]));
    }
    
    // Spot angles
    ImGui::SliderFloat("Inner Cutoff", &props.innerCutoff, 5.0f, 45.0f);
    ImGui::SliderFloat("Outer Cutoff", &props.outerCutoff, 10.0f, 60.0f);
    if (props.outerCutoff <= props.innerCutoff) {
        props.outerCutoff = props.innerCutoff + 5.0f;
    }
    
    ImGui::Text("Attenuation");
    ImGui::SliderFloat("Constant", &props.constant, 0.0f, 2.0f);
    ImGui::SliderFloat("Linear", &props.linear, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic", &props.quadratic, 0.0f, 1.0f);
    
    float range = light.calculateRange();
    ImGui::Text("Effective Range: %.1f units", range);
}

void ImGuiLightManager::addNewLight() {
    glm::vec3 color(newLightColor[0], newLightColor[1], newLightColor[2]);
    
    switch (newLightType) {
        case 0: { // Directional
            glm::vec3 direction(newLightDirection[0], newLightDirection[1], newLightDirection[2]);
            size_t index = lightManager.addDirectionalLight(direction, color, newLightIntensity);
            selectedLightIndex = static_cast<int>(index);
            break;
        }
        case 1: { // Point
            glm::vec3 position(newLightPosition[0], newLightPosition[1], newLightPosition[2]);
            size_t index = lightManager.addPointLight(position, color, newLightIntensity);
            selectedLightIndex = static_cast<int>(index);
            break;
        }
        case 2: { // Spot
            glm::vec3 position(newLightPosition[0], newLightPosition[1], newLightPosition[2]);
            glm::vec3 direction(newLightDirection[0], newLightDirection[1], newLightDirection[2]);
            size_t index = lightManager.addSpotLight(position, direction, color, newLightIntensity, newSpotInner, newSpotOuter);
            selectedLightIndex = static_cast<int>(index);
            break;
        }
    }
}

const char* ImGuiLightManager::getLightTypeName(LightType type) const {
    switch (type) {
        case LightType::Directional: return "Directional";
        case LightType::Point: return "Point";
        case LightType::Spot: return "Spot";
        default: return "Unknown";
    }
}

ImVec4 ImGuiLightManager::getLightTypeColor(LightType type) const {
    switch (type) {
        case LightType::Directional: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
        case LightType::Point: return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);       // Green
        case LightType::Spot: return ImVec4(0.0f, 0.5f, 1.0f, 1.0f);        // Blue
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);                     // White
    }
}