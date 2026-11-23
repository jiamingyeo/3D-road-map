/**
 * @file main.cpp
 */
#include <entt/entt.hpp>

#include "Component/CameraComponent.hpp"
#include "TestScene.hpp"
#include "shader.hpp"
#include "MapRenderer.hpp"
#include "Window.hpp"
#include "CoordinateConverter.hpp"

extern MapRenderer& GetMapRenderer();

static bool wKeyPressed = false;
static bool aKeyPressed = false;
static bool sKeyPressed = false;
static bool dKeyPressed = false;
static bool qKeyPressed = false;
static bool eKeyPressed = false;
static bool isRightMouseDown = false;
static bool isLeftMouseDown = false;
static double lastMouseX = 0.0;
static double lastMouseY = 0.0;
static bool firstMouse = true;

const float mouseSensitivity = 0.2f;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* reg = static_cast<entt::registry*>(glfwGetWindowUserPointer(window));
    if (!reg) return;

    auto view = reg->view<CameraComponent>();
    if (view.empty()) return;

    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

    switch (key) 
    {
    case GLFW_KEY_W: wKeyPressed = pressed; break;
    case GLFW_KEY_A: aKeyPressed = pressed; break;
    case GLFW_KEY_S: sKeyPressed = pressed; break;
    case GLFW_KEY_D: dKeyPressed = pressed; break;
    case GLFW_KEY_Q: qKeyPressed = pressed; break;
    case GLFW_KEY_E: eKeyPressed = pressed; break;
    case GLFW_KEY_MINUS:
        if (action == GLFW_PRESS) 
        {
            auto entity = *view.begin();
            auto& cam = reg->get<CameraComponent>(entity);
            cam.moveSpeed = std::max(10.0f, cam.moveSpeed * 0.8f);
        }
        break;
    case GLFW_KEY_EQUAL:
        if (action == GLFW_PRESS) {
            auto entity = *view.begin();
            auto& cam = reg->get<CameraComponent>(entity);
            cam.moveSpeed = std::min(1000.0f, cam.moveSpeed * 1.25f);
        }
        break;
    }
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto* reg = static_cast<entt::registry*>(glfwGetWindowUserPointer(window));
    if (!reg) return;

    auto view = reg->view<CameraComponent>();
    if (view.empty()) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) 
    {
        if (action == GLFW_PRESS) 
        {
            isRightMouseDown = true;
            firstMouse = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) 
        {
            isRightMouseDown = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
            isLeftMouseDown = true;
        }
        else if (action == GLFW_RELEASE)
        {
            isLeftMouseDown = false;
        }
    }
}

static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto* reg = static_cast<entt::registry*>(glfwGetWindowUserPointer(window));
    if (!reg) return;

    auto view = reg->view<CameraComponent>();
    if (view.empty()) return;

    if (!isRightMouseDown) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        return;
    }

    auto entity = *view.begin();
    auto& cam = reg->get<CameraComponent>(entity);
    double dx = xpos - lastMouseX;
    double dy = ypos - lastMouseY;

    cam.yaw += static_cast<float>(dx * mouseSensitivity);
    cam.pitch -= static_cast<float>(dy * mouseSensitivity);
    cam.pitch = glm::clamp(cam.pitch, -89.0f, 89.0f);

    lastMouseX = xpos;
    lastMouseY = ypos;
}

static void UpdateCameraMovement(CameraComponent& cam, float deltaTime)
{
    float speed = cam.moveSpeed * deltaTime;
    glm::vec3 forward = cam.GetForward();
    glm::vec3 right = cam.GetRight();

    forward.z = 0.0f;
    if (glm::length(forward) > 0.001f) 
    {
        forward = glm::normalize(forward);
    }

    if (wKeyPressed) cam.pos += forward * speed;
    if (sKeyPressed) cam.pos -= forward * speed;
    if (aKeyPressed) cam.pos -= right * speed;
    if (dKeyPressed) cam.pos += right * speed;
    if (qKeyPressed) cam.pos.z -= speed;
    if (eKeyPressed) cam.pos.z += speed;
    cam.pos.z = glm::clamp(cam.pos.z, 5.0f, 20000.0f);
}

int main() {
    Window::InitializeSystem();

    CoordinateConverter::Initialize(103.905, 1.405);

    Window window(1024, 768);
    entt::registry registry;

    glfwSetWindowUserPointer(window.GetGLFWWindow(), &registry);
    glfwSetMouseButtonCallback(window.GetGLFWWindow(), MouseButtonCallback);
    glfwSetCursorPosCallback(window.GetGLFWWindow(), CursorPosCallback);
    glfwSetKeyCallback(window.GetGLFWWindow(), KeyCallback);

    GLuint shaderProgram = LoadShaders(
        "../projects/jiaming.yeo-project-phase9/shaders/ColorVertexShader.vert",
        "../projects/jiaming.yeo-project-phase9/shaders/ColorFragmentShader.frag"
    );
    /*
    GLuint shaderProgram = LoadShaders(
        "../projects/YiQian.Lim-project-phase8/shaders/ColorVertexShader.vert",
        "../projects/YiQian.Lim-project-phase8/shaders/ColorFragmentShader.frag"
    );
    */
    
    TestScene testScene;
    testScene.Load(shaderProgram, registry, window);

    MapRenderer& mapRenderer = GetMapRenderer();

    while (!window.ShouldClose()) 
    {
        float dt = 0.016f;
        
        auto camView = registry.view<CameraComponent>();
        if (!camView.empty()) {
            auto entity = *camView.begin();
            auto& cam = registry.get<CameraComponent>(entity);

            UpdateCameraMovement(cam, dt);

            auto size = window.GetWindowSize();
            cam.UpdateMatrices(size.x, size.y);

            testScene.Update(shaderProgram, registry, window);
            if (isLeftMouseDown)
            {
                testScene.OnLeftClick(glm::vec2(lastMouseX , lastMouseY), window.GetWindowSize() , cam);
                isLeftMouseDown = false;
            }
            mapRenderer.Render(cam, window, dt);

            static int frameCount = 0;
            if (++frameCount % 30 == 0) 
            {
                char title[256];
                snprintf(
                    title,
                    sizeof(title),
                    "3D Road Map | pos (%.1f, %.1f, %.1f) | pitch %.1f yaw %.1f | speed %.0f | %d/%d segs bld %d poi %d | \xC2\xA9 OpenStreetMap contributors",
                    cam.pos.x,
                    cam.pos.y,
                    cam.pos.z,
                    cam.pitch,
                    cam.yaw,
                    cam.moveSpeed,
                    testScene.GetDrawnCount(),
                    testScene.GetSegmentCount(),
                    testScene.GetBuildingCount(),
                    testScene.GetPOICount(),
                    testScene.GetQueryCount());
                glfwSetWindowTitle(window.GetGLFWWindow(), title);
            }
        }
        
        window.Update();
    }

    window.DestroySystem();
    return 0;
}