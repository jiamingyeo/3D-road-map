#include "Skybox.hpp"

Skybox::Skybox() : mVAO(0), mShaderProgram(0)
{
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    glBindVertexArray(0);
}

Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &mVAO);
}

void Skybox::SetShaderProgram(GLuint program)
{
    mShaderProgram = program;
    if (program) 
    {
        uSkyColorLoc = glGetUniformLocation(mShaderProgram, "uSkyColor");
        uGroundColorLoc = glGetUniformLocation(mShaderProgram, "uGroundColor");
    }
}

void Skybox::Render(const glm::vec3& skyColor, const glm::vec3& groundColor)
{
    if (!mShaderProgram) return;

    glUseProgram(mShaderProgram);

    glUniform3fv(uSkyColorLoc, 1, &skyColor[0]);
    glUniform3fv(uGroundColorLoc, 1, &groundColor[0]);

    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glUseProgram(0);
}