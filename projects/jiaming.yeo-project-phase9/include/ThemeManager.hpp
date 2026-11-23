#pragma once

#include <vector>
#include <string>
#include <GLFW/glfw3.h>

#include "ColorTheme.hpp"

class ThemeManager 
{
public:
    ThemeManager() = default;

    bool Load(const std::string& jsonPath);
    const ColorTheme& Current() const { return mThemes[mCurrentIndex]; }
    void SetTheme(int index);
    bool HandleHotkeys(GLFWwindow* window);
    bool Changed() const { return mChangedThisFrame; }
    size_t Size() const { return mThemes.size(); }

private:
    std::vector<ColorTheme> mThemes;
    int mCurrentIndex = 0;
    bool mChangedThisFrame = false;
    bool mPrevKeyState[6] = { false, false, false, false, false, false };
    ColorTheme BuiltInFallback() const;
};