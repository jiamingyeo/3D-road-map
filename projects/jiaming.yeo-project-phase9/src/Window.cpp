/**
 * @file
 *  Window.cpp
 * @author
 *  Lim Yi Qian, 2202371, yiqianlim@digipen.edu
 * @date
 *  2024/05/21
 * @brief
 *	Definitions for Window class
 * @copyright
 *  Copyright (C) 2024 DigiPen Institute of Technology.
 */
#include "Window.hpp"

void Window::InitializeSystem()
{
	if (glfwInit() == 0)
	{
		return;
	}
	// Configure the OpenGL context version and profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS
}

Window::Window(int width, int height)
{
	// Create the main application window
	mWindow = glfwCreateWindow(1024, 768, "Render Cube", nullptr, nullptr);
	if (mWindow == NULL)
	{
		throw std::runtime_error("Could not create window");
	}

	// Set the current OpenGL context to the newly created window
	glfwMakeContextCurrent(mWindow);
	//glfwSwapInterval(1);

	// Enable sticky keys
	glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);

	// Initialize GLEW to load OpenGL functions
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) return;

	// Enable depth testing for proper 3D rendering
	glEnable(GL_DEPTH_TEST);
}

Window::Window(glm::vec2 windowSize, bool visible)
{
	if (visible) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	}
	mWindow = glfwCreateWindow(windowSize.x, windowSize.y, "A1", nullptr, nullptr);
	if (mWindow == NULL)
	{
		throw std::runtime_error("Could not create window");
	}
	glfwMakeContextCurrent(mWindow);
	//glfwSwapInterval(1);
	glfwShowWindow(mWindow);

	// Enable sticky keys
	glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);

	// Initialize GLEW to load OpenGL functions
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) return;

	// Enable depth testing for proper 3D rendering
	glEnable(GL_DEPTH_TEST);
}

void Window::DestroySystem()
{
	// Cleanly shut down and exit
	glfwTerminate();
}

void Window::ProcessEscape()
{
	if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(mWindow, true);
}

bool Window::ShouldClose() const
{
	return glfwWindowShouldClose(mWindow);
}

glm::vec2 Window::GetWindowSize() const
{
	int width = 0;
	int height = 0;
	glfwGetWindowSize(mWindow, &width, &height);
	return glm::vec2(width, height);
}

void Window::Update()
{
	glfwSwapBuffers(mWindow);  // Swap front and back buffers
	glfwPollEvents();          // Process input and events
}
