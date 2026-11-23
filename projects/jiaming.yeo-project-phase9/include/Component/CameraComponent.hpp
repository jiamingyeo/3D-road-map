#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

struct CameraComponent
{
    glm::vec3 pos = { 0.0f, 0.0f, 300.0f }; // world - metric position(wx, wy, altitude in meters)
    float pitch = -60.0f;
    float yaw = 0.0f;
    float fov = 100.0f;
    float moveSpeed = 100.0f;

    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::mat4 viewProjMatrix;

    void UpdateMatrices(int screenWidth, int screenHeight);
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
};