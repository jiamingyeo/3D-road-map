#include "buffer.hpp"

Buffer::Buffer()
{
    // Create 1 VAO
    glGenVertexArrays(1, &mVAO);
    // Create 1 VBO
    glGenBuffers(1, &mVBO);
    // Create 1 EBO
    glGenBuffers(1, &mEBO);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mEBO);
    glDeleteVertexArrays(1, &mVAO);
}

Buffer::Buffer(Buffer&& other) noexcept
    : mVAO(other.mVAO),
    mVBO(other.mVBO),
    mEBO(other.mEBO)
{
    other.mVAO = 0;
    other.mVBO = 0;
    other.mEBO = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {

    if (this != &other) {

        glDeleteVertexArrays(1, &mVAO);
        glDeleteBuffers(1, &mVBO);
        glDeleteBuffers(1, &mEBO);

        mVAO = other.mVAO;
        mVBO = other.mVBO;
        mEBO = other.mEBO;

        other.mVAO = 0;
        other.mVBO = 0;
        other.mEBO = 0;
    }

    return *this;
}

void Buffer::BindVAO() const
{
    glBindVertexArray(mVAO);
}

void Buffer::UnbindVAO() const
{
    glBindVertexArray(0);
}

void Buffer::BindVBO() const
{
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
}

void Buffer::BindEBO() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
}

void Buffer::UnbindVBO() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Buffer::SetData(GLsizeiptr size,
    const void* vertices,
    GLenum usage) const
{
    glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);
}

void Buffer::SetIndexData(GLsizeiptr size, const void* data, GLenum usage) const
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        size,
        data,
        usage
    );
}

void Buffer::SetAttribute(GLuint index,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLsizei stride,
    const void* offset) const
{
    glVertexAttribPointer(
        index,
        size,
        type,
        normalized,
        stride,
        offset);

    glEnableVertexAttribArray(index);
}
