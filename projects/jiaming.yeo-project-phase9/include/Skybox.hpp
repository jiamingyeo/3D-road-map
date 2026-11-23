#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class Skybox
{
public:
    Skybox();
    ~Skybox();

    void Render(const glm::vec3& skyColor, const glm::vec3& groundColor);
    void SetShaderProgram(GLuint program);

private:
    GLuint mVAO;
    GLuint mShaderProgram;
    GLuint uSkyColorLoc;
    GLuint uGroundColorLoc;
};