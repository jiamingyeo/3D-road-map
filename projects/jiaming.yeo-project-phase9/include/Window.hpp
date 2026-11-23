/**
 * @file
 *  Window.hpp
 * @author
 *  Lim Yi Qian, 2202371, yiqianlim@digipen.edu
 * @date
 *  2026/05/10
 * @brief
 *	Declarations for window class
 * @copyright
 *  Copyright (C) 2026 DigiPen Institute of Technology.
 */
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>

class Window {
public:
	Window(int width, int height);
	Window(glm::vec2 windowSize, bool visible);
	void Update();
	static void InitializeSystem();
	static void DestroySystem();
	void ProcessEscape();
	bool ShouldClose() const;

	glm::vec2 GetWindowSize() const;
	GLFWwindow* GetGLFWWindow() const { return mWindow; }

private:
	GLFWwindow* mWindow;
};