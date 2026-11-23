#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <GLFW/glfw3.h>

#include "ThemeManager.hpp"

bool ThemeManager::Load(const std::string& jsonPath)
{
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        printf("[ThemeManager] '%s' not found - using built-in fallback.\n", jsonPath.c_str());
        mThemes.push_back(BuiltInFallback());
        return false;
    }
    json root;
    try { root = json::parse(f); }
    catch (const json::parse_error& e) {
        printf("[ThemeManager] JSON parse error: %s\n", e.what());
        mThemes.push_back(BuiltInFallback());
        return false;
    }
    for (auto& t : root["themes"]) {
        ColorTheme ct;
        ct.id = t.value("id", 0);
        ct.name = t.value("name", std::string("Unknown"));
        ct.hotkey = t.value("hotkey", std::string("T+?"));
        ct.inspiredBy = t.value("inspired_by", std::string(""));
        ct.background = RGB::rgbFrom255(t["background"]);
        ct.skyground = RGB::rgbFrom255(t["skyground"]);
        for (auto& [k, v] : t["roads"].items()) ct.roads[k] = RGB::rgbFrom255(v);
        for (auto& [k, v] : t["buildings"].items()) ct.buildings[k] = RGB::rgbFrom255(v);
        for (auto& [k, v] : t["pois"].items()) ct.pois[k] = RGB::rgbFrom255(v);
        ct.pathInner = RGB::rgbFrom255(t["path"]["inner"]);
        ct.pathOutline = RGB::rgbFrom255(t["path"]["outline"]);
        auto& sun = t["sun"];
        ct.sunAmbient = sun.value("ambient", 0.25f);
        ct.sunElevation = sun.value("elevation", 45.0f);
        ct.sunAzimuth = sun.value("azimuth", 135.0f);
        mThemes.push_back(std::move(ct));
    }
    printf("[ThemeManager] Loaded %d themes.\n", (int)mThemes.size());
    return true;
}

void ThemeManager::SetTheme(int index)
{
    if (index >= 0 && index < static_cast<int>(mThemes.size()) && index != mCurrentIndex) 
    {
        mCurrentIndex = index;
        mChangedThisFrame = true;
        std::cout << "[ThemeManager] Switched to theme: " << mThemes[index].name << "\n";
    }
}

bool ThemeManager::HandleHotkeys(GLFWwindow* window)
{
    mChangedThisFrame = false;

    bool tHeld = (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS);
    if (!tHeld) 
    {
        std::fill(mPrevKeyState, mPrevKeyState + 6, false);
        return false;
    }

    const int numberKeys[6] = 
    {
        GLFW_KEY_1, 
        GLFW_KEY_2, 
        GLFW_KEY_3,
        GLFW_KEY_4,
        GLFW_KEY_5,
        GLFW_KEY_6
    };

    for (int i = 0; i < 6 && i < (int)mThemes.size(); ++i) 
    {
        bool now = (glfwGetKey(window, numberKeys[i]) == GLFW_PRESS);
        if (now && !mPrevKeyState[i])
        {
            SetTheme(i);
        }
        mPrevKeyState[i] = now;
    }
    return mChangedThisFrame;
}

ColorTheme ThemeManager::BuiltInFallback() const
{
    ColorTheme fallback;
    fallback.id = 0;
    fallback.name = "Fallback";
    fallback.background = RGB(0.15f, 0.15f, 0.15f);

    // Roads
    fallback.roads["motorway"] = RGB(1.0f, 0.76f, 0.0f);
    fallback.roads["trunk"] = RGB(1.0f, 0.55f, 0.2f);
    fallback.roads["primary"] = RGB(1.0f, 0.93f, 0.63f);
    fallback.roads["secondary"] = RGB(0.78f, 0.78f, 0.0f);
    fallback.roads["tertiary"] = RGB(0.7f, 0.7f, 0.7f);
    fallback.roads["residential"] = RGB(0.63f, 0.63f, 0.63f);
    fallback.roads["service"] = RGB(0.47f, 0.47f, 0.47f);
    fallback.roads["footway"] = RGB(0.31f, 0.78f, 0.31f);

    // Buildings
    fallback.buildings["residential"] = RGB(0.35f, 0.51f, 0.71f);
    fallback.buildings["apartments"] = RGB(0.27f, 0.43f, 0.63f);
    fallback.buildings["commercial"] = RGB(0.71f, 0.51f, 0.27f);
    fallback.buildings["industrial"] = RGB(0.63f, 0.39f, 0.31f);
    fallback.buildings["school"] = RGB(0.43f, 0.71f, 0.35f);
    fallback.buildings["hospital"] = RGB(0.86f, 0.31f, 0.31f);
    fallback.buildings["default"] = RGB(0.5f, 0.5f, 0.5f);

    // POIs
    fallback.pois["default"] = RGB(0.2f, 0.9f, 1.0f);

    // Path
    fallback.pathInner = RGB(1.0f, 0.2f, 0.2f);
    fallback.pathOutline = RGB(1.0f, 0.8f, 0.0f);

    // Sun
    fallback.sunAmbient = 0.25f;
    fallback.sunElevation = 45.0f;
    fallback.sunAzimuth = 135.0f;

    return fallback;
}
